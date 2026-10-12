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
#include "Playable.hpp"
#include "nsbconstants.hpp"
#include <SDL2/SDL.h>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#define STB_VORBIS_NO_PUSHDATA_API
#include "stb_vorbis.c"

namespace
{
const int DevRate = 44100;
const int DevChannels = 2;
const SDL_AudioFormat DevFormat = AUDIO_S16SYS;

SDL_AudioDeviceID Device = 0;
bool DeviceFailed = false;
std::vector<Playable*> Active;
}

// Accessors for non-Playable audio producers (Movie feeds its own stream).
int PlayableDevRate() { return DevRate; }
int PlayableDevChannels() { return DevChannels; }
bool PlayableHasDevice() { return Device != 0; }

void AudioCallback(void* Data, uint8_t* Out, int Len)
{
    memset(Out, 0, Len);
    // Temporary float mix buffer
    std::vector<float> Mix(Len / 2, 0.f);
    std::vector<int16_t> Tmp(Len / 2);

    for (Playable* P : Active)
    {
        if (!P->Playing || !P->Stream)
            continue;
        P->FeedStream();
        int Got = SDL_AudioStreamGet(P->Stream, Tmp.data(), Len);
        if (Got <= 0)
            continue;
        int Frames = Got / (DevChannels * 2);
        for (int i = 0; i < Frames * DevChannels; ++i)
            Mix[i] += Tmp[i] * P->Gain;
    }
    int16_t* OutS = (int16_t*)Out;
    for (size_t i = 0; i < Mix.size(); ++i)
    {
        float v = Mix[i];
        if (v > 32767.f) v = 32767.f;
        if (v < -32768.f) v = -32768.f;
        OutS[i] = (int16_t)v;
    }
}

void EnsureDevice()
{
    if (Device || DeviceFailed)
        return;
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0)
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec Want = {};
    Want.freq = DevRate;
    Want.format = DevFormat;
    Want.channels = DevChannels;
    Want.samples = 1024;
    Want.callback = AudioCallback;
    Device = SDL_OpenAudioDevice(nullptr, 0, &Want, nullptr, 0);
    if (Device)
        SDL_PauseAudioDevice(Device, 0);
    else
    {
        DeviceFailed = true;
        fprintf(stderr, "[audio] no output device, running silent\n");
    }
}

Playable::Playable(const string& FileName) : Playable()
{
    std::ifstream f(FileName, std::ios::binary);
    if (f)
    {
        Data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        int err;
        Vorbis = stb_vorbis_open_memory(Data.data(), Data.size(), &err, nullptr);
    }
    EnsureDevice();
}

Playable::Playable(Resource Res) : Playable()
{
    if (Res.IsValid())
    {
        Data.resize(Res.GetSize());
        char* p = Res.ReadData(0, Data.size());
        if (p)
        {
            memcpy(Data.data(), p, Data.size());
            delete[] p;
        }
        int err;
        Vorbis = stb_vorbis_open_memory(Data.data(), Data.size(), &err, nullptr);
    }
    EnsureDevice();
}

Playable::~Playable()
{
    if (Device)
    {
        SDL_LockAudioDevice(Device);
        for (auto i = Active.begin(); i != Active.end(); ++i)
            if (*i == this) { Active.erase(i); break; }
        SDL_UnlockAudioDevice(Device);
    }
    if (Stream)
        SDL_FreeAudioStream(Stream);
    if (Vorbis)
        stb_vorbis_close(Vorbis);
}

int32_t Playable::PositionMs()
{
    if (Device && SampleRate)
        return (int32_t)(DecodedSamples * 1000 / SampleRate);
    // No audio device: approximate position with wall clock.
    if (Playing)
        return (int32_t)(SDL_GetTicks() - PlayStartTicks);
    return 0;
}

int32_t Playable::RemainTime()
{
    int32_t Rem = DurationMs - PositionMs();
    return Rem > 0 ? Rem : 0;
}

int32_t Playable::DurationTime()
{
    return DurationMs;
}

int32_t Playable::PassageTime()
{
    return PositionMs();
}

void Playable::SetLoop(bool Loop)
{
    this->Loop = Loop;
    if (DeferredFastForwardPlay)
    {
        DeferredFastForwardPlay = false;
        if (Loop)
            Play();
    }
    if (getenv("NP_SND_DBG"))
        fprintf(stderr, "[snd] set-loop playable=%p bgm=%d loop=%d playing=%d\n",
                (void*)this, IsBGM, this->Loop, Playing);
}

void Playable::Stop()
{
    if (!Device)
    {
        Playing = false;
        if (getenv("NP_SND_DBG"))
            fprintf(stderr, "[snd] stop playable=%p bgm=%d playing=%d device=0\n",
                    (void*)this, IsBGM, Playing);
        return;
    }
    SDL_LockAudioDevice(Device);
    Playing = false;
    for (auto i = Active.begin(); i != Active.end(); ++i)
        if (*i == this) { Active.erase(i); break; }
    SDL_UnlockAudioDevice(Device);
    if (getenv("NP_SND_DBG"))
        fprintf(stderr, "[snd] stop playable=%p bgm=%d playing=%d device=1\n",
                (void*)this, IsBGM, Playing);
}

void Playable::StopAll()
{
    if (getenv("NP_SND_DBG"))
        fprintf(stderr, "[snd] stop-all active=%zu device=%d\n", Active.size(), !!Device);
    if (!Device)
    {
        for (Playable* P : Active)
            P->Playing = false;
        Active.clear();
        return;
    }
    SDL_LockAudioDevice(Device);
    for (Playable* P : Active)
        P->Playing = false;
    Active.clear();
    SDL_UnlockAudioDevice(Device);
}

void Playable::Play()
{
    if (Vorbis)
    {
        stb_vorbis_seek_start(Vorbis);
        DecodedSamples = 0;
        if (!Stream)
        {
            stb_vorbis_info Info = stb_vorbis_get_info(Vorbis);
            SampleRate = Info.sample_rate;
            Channels = Info.channels;
            DurationMs = (int32_t)(stb_vorbis_stream_length_in_samples(Vorbis) * 1000 / SampleRate);
            Stream = SDL_NewAudioStream(AUDIO_S16SYS, Channels, SampleRate,
                                      DevFormat, DevChannels, DevRate);
        }
        if (Stream)
            SDL_AudioStreamClear(Stream);
        if (LoopBeginMs > 0)
            stb_vorbis_seek(Vorbis, (uint64_t)LoopBeginMs * SampleRate / 1000);
    }
    PlayStartTicks = SDL_GetTicks();

    if (!Device)
    {
        Playing = true;
        if (getenv("NP_SND_DBG"))
            fprintf(stderr, "[snd] play playable=%p bgm=%d loop=%d playing=%d vorbis=%d stream=%d device=0\n",
                    (void*)this, IsBGM, Loop, Playing, !!Vorbis, !!Stream);
        return;
    }
    SDL_LockAudioDevice(Device);
    Playing = true;
    if (Vorbis || Stream)
    {
        bool Found = false;
        for (Playable* P : Active)
            if (P == this) { Found = true; break; }
        if (!Found)
            Active.push_back(this);
    }
    SDL_UnlockAudioDevice(Device);
    if (getenv("NP_SND_DBG"))
        fprintf(stderr, "[snd] play playable=%p bgm=%d loop=%d playing=%d vorbis=%d stream=%d device=1\n",
                (void*)this, IsBGM, Loop, Playing, !!Vorbis, !!Stream);
}

void Playable::FeedStream()
{
    // Called under the audio device lock.
    if (!Vorbis)
        return; // e.g. Movie, which feeds Stream from its own decoder
    const int ChunkFrames = 4096;
    std::vector<int16_t> Buf(ChunkFrames * Channels);
    int Produced = 0;
    while (Produced < ChunkFrames)
    {
        int Got = stb_vorbis_get_samples_short_interleaved(
            Vorbis, Channels, Buf.data() + Produced * Channels,
            (ChunkFrames - Produced) * Channels);
        if (Got <= 0)
        {
            if (Loop)
            {
                stb_vorbis_seek_start(Vorbis);
                DecodedSamples = 0;
                continue;
            }
            break;
        }
        Produced += Got;
        DecodedSamples += Got;
        if (LoopEndMs > 0 && PositionMs() >= LoopEndMs)
        {
            stb_vorbis_seek(Vorbis, (uint64_t)LoopBeginMs * SampleRate / 1000);
            DecodedSamples = LoopBeginMs * SampleRate / 1000;
            break;
        }
        if (!Loop && DecodedSamples * 1000 / SampleRate >= (uint64_t)DurationMs)
            break;
    }
    if (Produced > 0)
        SDL_AudioStreamPut(Stream, Buf.data(), Produced * Channels * 2);
}

void Playable::SetVolume(int32_t Time, int32_t Volume, int32_t Tempo)
{
    Gain = Volume / 1000.f;
}

void Playable::SetFrequency(int32_t Time, int32_t Frequency, int32_t Tempo)
{
}

void Playable::SetPan(int32_t Time, int32_t Pan, int32_t Tempo)
{
}

void Playable::SetLoopPoint(int32_t Begin, int32_t End)
{
    LoopBeginMs = Begin;
    LoopEndMs = End;
}

void Playable::Request(int32_t State)
{
    Object::Request(State);
    switch (State)
    {
        case Nsb::RESUME:
        case Nsb::PLAY:
            Play();
            break;
        case Nsb::PAUSE:
            Stop();
            break;
    }
}

// [HACK]
bool Playable::Action()
{
    return !Playing || RemainTime() <= 0;
}
