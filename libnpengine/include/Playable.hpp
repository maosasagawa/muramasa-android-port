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
#ifndef PLAYABLE_HPP
#define PLAYABLE_HPP

#include "Object.hpp"
#include "ResourceMgr.hpp"
#include <vector>

struct stb_vorbis;
typedef struct _SDL_AudioStream SDL_AudioStream;

class Playable : virtual public Object
{
public:
    Playable(const string& FileName);
    Playable(Resource Res);
    virtual ~Playable();

    void SetVolume(int32_t Time, int32_t Volume, int32_t Tempo);
    void SetFrequency(int32_t Time, int32_t Frequency, int32_t Tempo);
    void SetPan(int32_t Time, int32_t Pan, int32_t Tempo);
    void SetLoopPoint(int32_t Begin, int32_t End);
    void SetLoop(bool Loop);
    void Stop();
    static void StopAll();
    void Play();
    int32_t RemainTime();
    int32_t DurationTime();
    int32_t PassageTime();
    void DeferFastForwardPlay() { DeferredFastForwardPlay = true; }
    void ClearDeferredFastForwardPlay() { DeferredFastForwardPlay = false; }
    void OnEOS() { }
    void Request(int32_t State);
    virtual bool Action();
    bool IsBGM;

protected:
    Playable() : Vorbis(nullptr), Stream(nullptr), Playing(false), Loop(false),
                 DeferredFastForwardPlay(false), Gain(1.f), DurationMs(0),
                 DecodedSamples(0), LoopBeginMs(0), LoopEndMs(0), SampleRate(0),
                 Channels(0), PlayStartTicks(0), IsBGM(false) { }

    // Decodes more audio into the conversion stream. Called from the mixer.
    void FeedStream();
    int32_t PositionMs();

    std::vector<uint8_t> Data;
    stb_vorbis* Vorbis;
    SDL_AudioStream* Stream;
    bool Playing;
    bool Loop;
    bool DeferredFastForwardPlay;
    float Gain;
    int32_t DurationMs;
    uint64_t DecodedSamples;
    int32_t LoopBeginMs, LoopEndMs;
    int SampleRate, Channels;
    uint32_t PlayStartTicks;

    friend void AudioCallback(void* Data, uint8_t* Out, int Len);
};

#endif
