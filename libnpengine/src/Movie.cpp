/*
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2013-2016,2018 Mislav Blažević <krofnica996@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 *
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#include "Movie.hpp"
#include "Window.hpp"
#include <SDL_audio.h>
#include <fstream>
#ifdef __ANDROID__
#include <android/log.h>
#define MVLOG(...) __android_log_print(ANDROID_LOG_INFO, "muramasa", __VA_ARGS__)
#else
#define MVLOG(...) do { if (getenv("NP_MOVIE_DBG")) fprintf(stderr, __VA_ARGS__); } while (0)
#endif

int PlayableDevRate();
int PlayableDevChannels();
bool PlayableHasDevice();
void EnsureDevice();

Movie::Movie(const string& FileName, Window* pWindow, int32_t Priority, bool Alpha, bool Audio) :
Alpha(Alpha), HasAudio(Audio), Plm(nullptr), Hidden(false), FrameW(0), FrameH(0),
NewFrame(false), PushedSamples(0), WaitTicks(0), AutoHide(false), HadWaiter(false)
{
    SetPriority(Priority);
    EnsureDevice();

    std::ifstream f(FileName, std::ios::binary);
    if (!f)
        f.open(std::string("./") + FileName, std::ios::binary);
    if (!f)
    {
        MVLOG("[movie] cannot open %s\n", FileName.c_str());
        return;
    }
    Data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());

    Plm = plm_create_with_memory(Data.data(), Data.size(), 0);
    if (!Plm)
    {
        MVLOG("[movie] plm_create failed for %s\n", FileName.c_str());
        return;
    }
    if (plm_get_num_video_streams(Plm) < 1)
    {
        MVLOG("[movie] no video stream in %s\n", FileName.c_str());
        plm_destroy(Plm);
        Plm = nullptr;
        return;
    }

    FrameW = plm_get_width(Plm);
    FrameH = plm_get_height(Plm);
    FrameBuf.resize(FrameW * FrameH * 3);
    plm_set_video_decode_callback(Plm, &Movie::OnVideo, this);
    plm_set_audio_lead_time(Plm, 0.4);
    plm_set_loop(Plm, Loop);

    double Dur = plm_get_duration(Plm);
    DurationMs = Dur > 0 ? (int32_t)(Dur * 1000.0) : 0;
    MVLOG("[movie] %s %dx%d dur=%dms video=%d audio=%d\n",
          FileName.c_str(), FrameW, FrameH, DurationMs,
          plm_get_num_video_streams(Plm), plm_get_num_audio_streams(Plm));

    if (HasAudio && plm_get_num_audio_streams(Plm) > 0)
    {
        SampleRate = plm_get_samplerate(Plm);
        Channels = 2;
        Stream = SDL_NewAudioStream(AUDIO_S16SYS, Channels, SampleRate,
                                  AUDIO_S16SYS, PlayableDevChannels(), PlayableDevRate());
        plm_set_audio_decode_callback(Plm, &Movie::OnAudio, this);
    }
    else
    {
        plm_set_audio_enabled(Plm, 0);
        HasAudio = false;
    }
}

Movie::~Movie()
{
    if (Plm)
        plm_destroy(Plm);
}

void Movie::Play()
{
    if (Plm)
    {
        plm_set_loop(Plm, Loop);
        PushedSamples = 0;
        NewFrame = false;
        Hidden = false;
        HadWaiter = false;
        WaitTicks = SDL_GetTicks();
        if (Stream)
            SDL_AudioStreamClear(Stream);
    }
    Playable::Play();
}

int32_t Movie::MoviePositionMs()
{
    if (PlayableHasDevice() && Stream && SampleRate)
    {
        // Queued bytes are in output-rate samples; convert to ms before
        // subtracting from the source-rate push clock.
        int QueuedMs = SDL_AudioStreamAvailable(Stream) /
                       (PlayableDevChannels() * 2) * 1000 / PlayableDevRate();
        int64_t Pos = (int64_t)PushedSamples * 1000 / SampleRate - QueuedMs;
        if (Pos < 0)
            Pos = 0;
        return (int32_t)Pos;
    }
    if (Playing)
        return (int32_t)(SDL_GetTicks() - PlayStartTicks);
    return 0;
}

int32_t Movie::RemainTime()
{
    if (!Plm)
        return Playable::RemainTime();
    if (!Playing)
        return 0;
    int32_t Pos = MoviePositionMs();
    int32_t Rem = DurationMs - Pos;
    return Rem > 0 ? Rem : 0;
}

int32_t Movie::DurationTime()
{
    return DurationMs;
}

int32_t Movie::PassageTime()
{
    return MoviePositionMs();
}

void Movie::Draw(uint32_t Diff)
{
    if (AutoHide && Playing && !Waiters.empty())
        HadWaiter = true;

    if (AutoHide && ((Playing && HadWaiter && Waiters.empty()) ||
                     (Plm && plm_has_ended(Plm) && !Loop)))
    {
        Stop();
        Hidden = true;
        MVLOG("[movie] hidden (ended or skipped)\n");
    }

    if (Plm && Playing && !plm_has_ended(Plm))
    {
        plm_decode(Plm, Diff / 1000.0);
        if (NewFrame)
        {
            Create(FrameBuf.data(), GL_RGB, FrameW, FrameH);
            NewFrame = false;
        }
    }
    if (!Hidden)
        Texture::Draw(Diff);
}

void Movie::OnVideo(plm_t*, plm_frame_t* Frame, void* User)
{
    Movie* M = static_cast<Movie*>(User);
    plm_frame_to_rgb(Frame, M->FrameBuf.data(), M->FrameW * 3);
    M->NewFrame = true;
}

void Movie::OnAudio(plm_t*, plm_samples_t* Samples, void* User)
{
    Movie* M = static_cast<Movie*>(User);
    int16_t Buf[PLM_AUDIO_SAMPLES_PER_FRAME * 2];
    for (unsigned i = 0; i < Samples->count * 2; ++i)
    {
        float v = Samples->interleaved[i] * 32767.f;
        if (v > 32767.f) v = 32767.f;
        if (v < -32768.f) v = -32768.f;
        Buf[i] = (int16_t)v;
    }
    M->PushedSamples += Samples->count;
    if (M->Stream && PlayableHasDevice())
        SDL_AudioStreamPut(M->Stream, Buf, Samples->count * 2 * sizeof(int16_t));
}
