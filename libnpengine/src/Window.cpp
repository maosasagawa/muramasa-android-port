/*
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014-2016,2018 Mislav Blažević <krofnica996@gmail.com>
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
#ifndef NP_GLES2
#include <GL/glew.h>
#endif
#include "GLUtil.hpp"
#include <iostream>
#include "NSBInterpreter.hpp"
#include "Window.hpp"
#include "Texture.hpp"

uint32_t SDL_NSB_MOVECURSOR;
Window* Object::pWindow = nullptr;

Window::Window(const char* WindowTitle, const int Width, const int Height) : WIDTH(Width), HEIGHT(Height), pInterpreter(nullptr), IsRunning(true), EventLoop(false)
{
    Object::pWindow = this;
    SDL_Init(SDL_INIT_VIDEO);
#ifdef NP_GLES2
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif
    SDLWindow = SDL_CreateWindow(WindowTitle, 0, 0, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    GLContext = SDL_GL_CreateContext(SDLWindow);
    SDL_NSB_MOVECURSOR = SDL_RegisterEvents(1);

#ifndef NP_GLES2
    GLenum err = glewInit();
    if (err != GLEW_OK)
        cout << glewGetErrorString(err) << endl;
#endif

    GLInit(WIDTH, HEIGHT);
    int SurfW = WIDTH, SurfH = HEIGHT;
    SDL_GetWindowSize(SDLWindow, &SurfW, &SurfH);
    GLInitViewport(SurfW, SurfH);
}

Window::~Window()
{
    SDL_GL_DeleteContext(GLContext);
    SDL_DestroyWindow(SDLWindow);
    SDL_Quit();
    delete pInterpreter;
}

void Window::PushMoveCursorEvent(int X, int Y)
{
    SDL_Event Event;
    SDL_zero(Event);
    Event.type = SDL_NSB_MOVECURSOR;
    Event.user.data1 = reinterpret_cast<void*>(X);
    Event.user.data2 = reinterpret_cast<void*>(Y);
    SDL_PushEvent(&Event);
}

void Window::Run()
{
    LastDrawTime = SDL_GetTicks();
    SDL_Event Event;
    while (IsRunning)
    {
        while (SDL_PollEvent(&Event))
            HandleEvent(Event);

        Draw();
        pInterpreter->Run(pInterpreter->IsFastForwarding() ? 5000 : 100);
        SDL_Delay(10);
    }
}

void Window::Exit()
{
    IsRunning = false;
}

void Window::Select(bool Enable)
{
    EventLoop = Enable;
}

void Window::HandleEvent(SDL_Event& Event)
{
    if (Event.type == SDL_APP_WILLENTERBACKGROUND)
        pInterpreter->OnBackground();
    // Map window-pixel coords into game coordinates (letterboxed on Android).
    if (Event.type == SDL_MOUSEBUTTONDOWN || Event.type == SDL_MOUSEBUTTONUP)
        GLSurfaceToGame(Event.button.x, Event.button.y);
    else if (Event.type == SDL_MOUSEMOTION)
        GLSurfaceToGame(Event.motion.x, Event.motion.y);

    if (Event.type == SDL_NSB_MOVECURSOR)
        MoveCursor((int64_t)Event.user.data1, (int64_t)Event.user.data2);
    else if (EventLoop)
    {
        if (getenv("NP_CHOICE_DBG") && Event.type != SDL_MOUSEMOTION)
            fprintf(stderr, "[win] push event type=%x\n", Event.type);
        pInterpreter->PushEvent(Event);
    }
    else if (getenv("NP_CHOICE_DBG") && Event.type != SDL_MOUSEMOTION)
        fprintf(stderr, "[win] event type=%x dropped (no select)\n", Event.type);

    pInterpreter->HandleEvent(Event);
}

void Window::Draw()
{
    uint32_t CurrTime = SDL_GetTicks();
    uint32_t Diff = CurrTime - LastDrawTime;
    DrawTextures(Diff);
    pInterpreter->Update(Diff);
    SDL_GL_SwapWindow(SDLWindow);
    LastDrawTime = CurrTime;
}

void Window::DrawTextures(uint32_t Diff)
{
    glClear(GL_COLOR_BUFFER_BIT);
    for (Texture* pTex : Textures)
        pTex->Draw(Diff);
    if (getenv("NP_TEX_DBG"))
    {
        static size_t Frame = 0;
        ++Frame;
        for (Texture* t : Textures)
        {
            const string& Handle = t->DebugName;
            if (Handle.find("Config1a_TDir") == string::npos &&
                Handle.find("Config1a_TateON") == string::npos &&
                Handle.find("Config1a_YokoON") == string::npos &&
                Handle != "boxH" && Handle != "boxTH" &&
                Handle != "boxCC" && Handle != "boxCT" &&
                Handle.find("/text") == string::npos &&
                Handle.find("FwH") == string::npos &&
                Handle.find("FwC") == string::npos &&
                Handle.find("FwR") == string::npos &&
                Handle.find("FwL") == string::npos &&
                Handle != "絵色100" && Handle != "黒幕１")
                continue;
            int Width = t->GetWidth();
            int Height = t->GetHeight();
            int Alpha = t->DebugAlpha();
            int Visible = Alpha > 0 && Width > 0 && Height > 0;
            fprintf(stderr, "[tex-draw] frame=%zu handle=%s priority=%d pos=%d,%d size=%dx%d alpha=%d visible=%d\n",
                    Frame, Handle.c_str(), t->GetPriority(),
                    t->GetX(), t->GetY(), Width, Height, Alpha, Visible);
        }
    }
    if (getenv("NP_DRAW_DBG"))
    {
        static int frames = 0;
        if (++frames % 60 == 0)
        {
            fprintf(stderr, "[draw] %zu textures glerr=%x\n", Textures.size(), glGetError());
            int i = 0;
            for (Texture* t : Textures)
                fprintf(stderr, "  [%d] %s X=%d Y=%d W=%d H=%d a=%d\n", i++,
                        t->DebugName.c_str(), t->GetX(), t->GetY(), t->GetWidth(), t->GetHeight(),
                        t->DebugAlpha());
            uint8_t px[4] = {0};
            glReadPixels(WIDTH/2, HEIGHT/2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
            fprintf(stderr, "  center pixel: %d %d %d %d\n", px[0], px[1], px[2], px[3]);
            Texture* ft = Textures.front();
            glBindTexture(GL_TEXTURE_2D, ft->GetID());
#ifndef NP_GLES2
            uint8_t* tp = new uint8_t[ft->GetWidth() * ft->GetHeight() * 4];
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, tp);
            unsigned long tsum = 0;
            for (int k = 0; k < ft->GetWidth()*ft->GetHeight()*4; k += 4099) tsum += tp[k];
            fprintf(stderr, "  front tex id=%d err=%x sample=%lu\n", ft->GetID(), glGetError(), tsum);
            delete[] tp;
#endif
        }
    }
}

void Window::AddTexture(Texture* pTexture)
{
    auto Spot = Textures.begin();
    while (Spot != Textures.end())
    {
        if ((*Spot)->GetPriority() > pTexture->GetPriority())
            break;
        ++Spot;
    }
    Textures.insert(Spot, pTexture);
}

void Window::RemoveTexture(Texture* pTexture)
{
    if (getenv("NP_TEX_DBG"))
        fprintf(stderr, "[tex] RemoveTexture %s\n", pTexture->DebugName.c_str());
    pTexture->UpdateEffects(0);
    Textures.remove(pTexture);
}

void Window::MoveCursor(int X, int Y)
{
    SDL_WarpMouseInWindow(SDLWindow, X, Y);
}

void Window::SetFullscreen(Uint32 Flags)
{
    SDL_SetWindowFullscreen(SDLWindow, Flags);
}
