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
#ifndef MOVIE_HPP
#define MOVIE_HPP

#include "Playable.hpp"
#include "Texture.hpp"

extern "C" {
#include "plm/pl_mpeg.h"
}

// MPEG-1 PS (.ngs) playback via pl_mpeg. Decoding happens on the render
// thread inside Draw(): plm_decode is fed the frame delta, video frames go
// to the GL texture and MP2 audio to this object's SDL_AudioStream through
// Playable's mixing path (Vorbis stays null).
class Movie : public Playable, public Texture
{
public:
    Movie(const string& FileName, Window* pWindow, int32_t Priority, bool Alpha, bool Audio);
    ~Movie();

    virtual void Request(int32_t State) { Playable::Request(State); }
    void Draw(uint32_t Diff);
    void Play();
    int32_t RemainTime();
    int32_t DurationTime();
    int32_t PassageTime();
    bool HasVideo() const { return Plm != nullptr; }
private:
    int32_t MoviePositionMs();

    static void OnVideo(plm_t* Self, plm_frame_t* Frame, void* User);
    static void OnAudio(plm_t* Self, plm_samples_t* Samples, void* User);

    bool Alpha;
    bool HasAudio;
public:
    // MoviePlay-mode: hide and stop as soon as the stream ends or every
    // waiter context abandoned its wait (click-skip).
    bool AutoHide;
private:
    bool HadWaiter;
    plm_t* Plm;
    bool Hidden;
    std::vector<uint8_t> FrameBuf;
    int FrameW, FrameH;
    bool NewFrame;
    uint64_t PushedSamples;
    uint32_t WaitTicks;
};

#endif
