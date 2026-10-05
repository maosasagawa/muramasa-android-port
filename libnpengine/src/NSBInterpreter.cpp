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
#include <unistd.h>
#include <cassert>
#include "NSBInterpreter.hpp"
#include "NSBContext.hpp"
#include "Texture.hpp"
#include "Image.hpp"
#include "Window.hpp"
#include "Movie.hpp"
#include "Text.hpp"
#include "Scrollbar.hpp"
#include "nsbmagic.hpp"
#include "nsbconstants.hpp"
#include "scriptfile.hpp"
#ifdef __ANDROID__
#include <SDL_system.h>
#endif
#include "npafile.hpp"
#include "fscommon.hpp"
#include "buffer.hpp"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <memory>
#include <algorithm>
#include <set>
#include <png.h>
#include <cctype>

#define NSB_ERROR(MSG1, MSG2) cout << __PRETTY_FUNCTION__ << ": " << MSG1 << " " << MSG2 << endl;
#define NSB_VARARGS 0xFF

namespace
{
bool IsMenuVisibilityVariable(const string& Name)
{
    static const set<string> Variables =
    {
        "$SYSTEM_menu_enable",
        "$SYSTEM_menu_save_enable",
        "$SYSTEM_menu_load_enable",
        "$SYSTEM_menu_config_enable",
        "$SYSTEM_backlog_enable",
        "$SYSTEM_backselect_enable",
        "$SYSTEM_menu_close_enable",
        "$SYSTEM_reset_enable"
    };
    return Variables.count(Name) != 0;
}

bool IsDialogueWindowTexture(const string& Name)
{
    static const set<string> Textures =
    {
        "boxH", "boxTH", "boxCC", "boxCT", "boxRR", "boxRT", "boxLL", "boxLT"
    };
    return Textures.count(Name) != 0;
}

bool IsHorizontalTypeBegin(const string& Symbol)
{
    size_t SideIndex = 9;
    if (Symbol.compare(0, 13, "TypeBeginTime") == 0)
        SideIndex = 13;
    else if (Symbol.compare(0, 10, "TypeBeginA") == 0)
        SideIndex = 10;
    else if (Symbol.compare(0, 9, "TypeBegin") != 0)
        return false;

    return Symbol.size() > SideIndex && Symbol[SideIndex] == 'H';
}

char GetTypeBeginSide(const string& Symbol)
{
    if (Symbol.compare(0, 10, "TypeBeginA") == 0)
        return 'C';

    size_t SideIndex = Symbol.compare(0, 13, "TypeBeginTime") == 0 ? 13 : 9;
    return Symbol.size() > SideIndex ? Symbol[SideIndex] : 'C';
}

bool ResolveFunction(NSBContext* Context, const string& Symbol)
{
    ScriptFile* pScript = Context->GetScript();
    if (pScript && pScript->GetSymbol(Symbol) != NSB_INVALIDE_LINE)
        return true;
    uint32_t CodeLine;
    return sResourceMgr->ResolveSymbol(Symbol, CodeLine) != nullptr;
}

bool IsTypeBeginSuffix(const string& Suffix)
{
    return Suffix == "I" || Suffix == "O" || Suffix == "IO" || Suffix == "IFO";
}

bool GetHorizontalRedirect(const string& Symbol, string& Target, bool& DropLast, bool& AppendEmpty)
{
    static const map<string, string> Direct =
    {
        {"SetTextEXC", "SetTextEXH"},
        {"SetTextEXR", "SetTextEXH"},
        {"SetTextEXL", "SetTextEXH"},
        {"SetTextA", "SetTextEXAH"},
        {"SetFwC", "SetFwH"},
        {"SetFwR", "SetFwH"},
        {"SetFwL", "SetFwH"},
        {"SetNwC", "SetFwH"},
        {"SetNwR", "SetFwH"},
        {"SetNwL", "SetFwH"},
        {"FwC", "FwH"},
        {"FwR", "FwH"},
        {"FwL", "FwH"},
        {"NwC", "NwH"},
        {"NwR", "NwH"},
        {"NwL", "NwH"},
        {"FadeFwC", "FadeFwH"},
        {"FadeFwR", "FadeFwH"},
        {"FadeFwL", "FadeFwH"},
        {"DeleteFwC", "DeleteFwH"},
        {"DeleteFwR", "DeleteFwH"},
        {"DeleteFwL", "DeleteFwH"}
    };
    auto Match = Direct.find(Symbol);
    if (Match != Direct.end())
    {
        Target = Match->second;
        DropLast = Symbol == "FwC";
        AppendEmpty = Symbol == "SetNwC" || Symbol == "SetNwR" || Symbol == "SetNwL";
        return true;
    }

    const string TimePrefix = "TypeBeginTime";
    if (Symbol.compare(0, TimePrefix.size(), TimePrefix) == 0 &&
        Symbol.size() > TimePrefix.size() + 1 &&
        (Symbol[TimePrefix.size()] == 'C' || Symbol[TimePrefix.size()] == 'R' ||
         Symbol[TimePrefix.size()] == 'L'))
    {
        string Suffix = Symbol.substr(TimePrefix.size() + 1);
        if (IsTypeBeginSuffix(Suffix))
        {
            Target = TimePrefix + "H" + Suffix;
            return true;
        }
    }

    const string Prefix = "TypeBegin";
    if (Symbol.compare(0, Prefix.size(), Prefix) == 0 &&
        Symbol.size() > Prefix.size() + 1 &&
        (Symbol[Prefix.size()] == 'C' || Symbol[Prefix.size()] == 'R' ||
         Symbol[Prefix.size()] == 'L'))
    {
        string Suffix = Symbol.substr(Prefix.size() + 1);
        if (IsTypeBeginSuffix(Suffix))
        {
            Target = Prefix + "H" + Suffix;
            return true;
        }
    }

    const string AllPrefix = "TypeBeginA";
    if (Symbol.compare(0, AllPrefix.size(), AllPrefix) == 0)
    {
        string Suffix = Symbol.substr(AllPrefix.size());
        if (Suffix == "I" || Suffix == "O" || Suffix == "IO")
        {
            Target = AllPrefix + "H" + Suffix;
            return true;
        }
    }
    return false;
}
}

static void LogMissingScrollbar(const string& Handle)
{
    static set<string> LoggedHandles;
    if (getenv("NP_TEX_DBG") && LoggedHandles.insert(Handle).second)
        fprintf(stderr, "[scrollbar] missing handle=%s\n", Handle.c_str());
}

NSBInterpreter::NSBInterpreter(Window* pWindow) :
pDebuggerThread(nullptr),
LogCalls(false),
DbgStepping(false),
RunInterpreter(true),
SkipHack(false),
pWindow(pWindow),
pContext(nullptr),
Builtins(MAGIC_UNK119 + 1, {nullptr, 0}),
pBootScript(nullptr)
{
    srand(time(0));

    // Engine-level default: real engine provides a save dir.
#ifdef __ANDROID__
    SetVar("#SYSTEM_save_path", Variable::MakeString(SDL_AndroidGetInternalStoragePath()));
#else
    SetVar("#SYSTEM_save_path", Variable::MakeString("."));
#endif
    SetString("#SYSTEM_text_horizontal", "false");

    Builtins[MAGIC_FUNCTION_DECLARATION] = { &NSBInterpreter::FunctionDeclaration, 0 };
    Builtins[MAGIC_CALL_FUNCTION] = { &NSBInterpreter::CallFunction, 0 };
    Builtins[MAGIC_CALL_SCENE] = { &NSBInterpreter::CallScene, 0 };
    Builtins[MAGIC_CALL_CHAPTER] = { &NSBInterpreter::CallChapter, 0 };
    Builtins[MAGIC_CMP_LOGICAL_AND] = { &NSBInterpreter::CmpLogicalAnd, 2 };
    Builtins[MAGIC_CMP_LOGICAL_OR] = { &NSBInterpreter::CmpLogicalOr, 2 };
    Builtins[MAGIC_CMP_GE] = { &NSBInterpreter::CmpGE, 2 };
    Builtins[MAGIC_CMP_LE] = { &NSBInterpreter::CmpLE, 2 };
    Builtins[MAGIC_CMP_GREATER] = { &NSBInterpreter::CmpGreater, 2 };
    Builtins[MAGIC_CMP_LESS] = { &NSBInterpreter::CmpLess, 2 };
    Builtins[MAGIC_CMP_EQUAL] = { &NSBInterpreter::CmpEqual, 2 };
    Builtins[MAGIC_CMP_NE] = { &NSBInterpreter::CmpNE, 2 };
    Builtins[MAGIC_NOT_EXPRESSION] = { &NSBInterpreter::NotExpression, 1 };
    Builtins[MAGIC_ADD_EXPRESSION] = { &NSBInterpreter::AddExpression, 2 };
    Builtins[MAGIC_SUB_EXPRESSION] = { &NSBInterpreter::SubExpression, 2 };
    Builtins[MAGIC_MUL_EXPRESSION] = { &NSBInterpreter::MulExpression, 2 };
    Builtins[MAGIC_DIV_EXPRESSION] = { &NSBInterpreter::DivExpression, 2 };
    Builtins[MAGIC_MOD_EXPRESSION] = { &NSBInterpreter::ModExpression, 2 };
    Builtins[MAGIC_INCREMENT] = { &NSBInterpreter::Increment, 1 };
    Builtins[MAGIC_DECREMENT] = { &NSBInterpreter::Decrement, 1 };
    Builtins[MAGIC_LITERAL] = { &NSBInterpreter::Literal, 0 };
    Builtins[MAGIC_ASSIGN] = { &NSBInterpreter::Assign, 1 };
    Builtins[MAGIC_VARIABLE] = { &NSBInterpreter::Get, 0 };
    Builtins[MAGIC_SCOPE_BEGIN] = { &NSBInterpreter::ScopeBegin, 0 };
    Builtins[MAGIC_SCOPE_END] = { &NSBInterpreter::ScopeEnd, 0 };
    Builtins[MAGIC_RETURN] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_END_FUNCTION] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_END_SCENE] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_END_CHAPTER] = { &NSBInterpreter::Return, 0 };
    Builtins[MAGIC_IF] = { &NSBInterpreter::If, 1 };
    Builtins[MAGIC_WHILE] = { &NSBInterpreter::While, 1 };
    Builtins[MAGIC_WHILE_END] = { &NSBInterpreter::WhileEnd, 0 };
    Builtins[MAGIC_SELECT] = { &NSBInterpreter::Select, 0 };
    Builtins[MAGIC_SELECT_END] = { &NSBInterpreter::SelectEnd, 0 };
    Builtins[MAGIC_SELECT_BREAK_END] = { &NSBInterpreter::SelectBreakEnd, 0 };
    Builtins[MAGIC_BREAK] = { &NSBInterpreter::Break, 0 };
    Builtins[MAGIC_JUMP] = { &NSBInterpreter::Jump, 0 };
    Builtins[MAGIC_ADD_ASSIGN] = { &NSBInterpreter::AddAssign, 1 };
    Builtins[MAGIC_SUB_ASSIGN] = { &NSBInterpreter::SubAssign, 1 };
    Builtins[MAGIC_MOD_ASSIGN] = { &NSBInterpreter::ModAssign, 1 };
    Builtins[MAGIC_WRITE_FILE] = { &NSBInterpreter::WriteFile, 2 };
    Builtins[MAGIC_READ_FILE] = { &NSBInterpreter::ReadFile, 1 };
    Builtins[MAGIC_CREATE_TEXTURE] = { &NSBInterpreter::CreateTexture, 5 };
    Builtins[MAGIC_IMAGE_HORIZON] = { &NSBInterpreter::ImageHorizon, 1 };
    Builtins[MAGIC_IMAGE_VERTICAL] = { &NSBInterpreter::ImageVertical, 1 };
    Builtins[MAGIC_TIME] = { &NSBInterpreter::Time, 0 };
    Builtins[MAGIC_STR_STR] = { &NSBInterpreter::StrStr, 2 };
    Builtins[MAGIC_EXIT] = { &NSBInterpreter::Exit, 0 };
    Builtins[MAGIC_CURSOR_POSITION] = { &NSBInterpreter::CursorPosition, 2 };
    Builtins[MAGIC_MOVE_CURSOR] = { &NSBInterpreter::MoveCursor, 2 };
    Builtins[MAGIC_POSITION] = { &NSBInterpreter::Position, 3 };
    Builtins[MAGIC_WAIT] = { &NSBInterpreter::Wait, 1 };
    Builtins[MAGIC_WAIT_KEY] = { &NSBInterpreter::WaitKey, NSB_VARARGS };
    Builtins[MAGIC_NEGA_EXPRESSION] = { &NSBInterpreter::NegaExpression, 1 };
    Builtins[MAGIC_SYSTEM] = { &NSBInterpreter::System, 3 };
    Builtins[MAGIC_STRING] = { &NSBInterpreter::String, NSB_VARARGS };
    Builtins[MAGIC_VARIABLE_VALUE] = { &NSBInterpreter::VariableValue, NSB_VARARGS };
    Builtins[MAGIC_CREATE_PROCESS] = { &NSBInterpreter::CreateProcess, 5 };
    Builtins[MAGIC_COUNT] = { &NSBInterpreter::Count, 1 };
    Builtins[MAGIC_ARRAY] = { &NSBInterpreter::Array, NSB_VARARGS };
    Builtins[MAGIC_SUB_SCRIPT] = { &NSBInterpreter::SubScript, 0 };
    Builtins[MAGIC_ASSOC_ARRAY] = { &NSBInterpreter::AssocArray, NSB_VARARGS };
    Builtins[MAGIC_MODULE_FILE_NAME] = { &NSBInterpreter::ModuleFileName, 0 };
    Builtins[MAGIC_REQUEST] = { &NSBInterpreter::Request, 2 };
    Builtins[MAGIC_SET_VERTEX] = { &NSBInterpreter::SetVertex, 3 };
    Builtins[MAGIC_ZOOM] = { &NSBInterpreter::Zoom, 6 };
    Builtins[MAGIC_MOVE] = { &NSBInterpreter::Move, 6 };
    Builtins[MAGIC_SET_SHADE] = { &NSBInterpreter::SetShade, 2 };
    Builtins[MAGIC_DRAW_TO_TEXTURE] = { &NSBInterpreter::DrawToTexture, 4 };
    Builtins[MAGIC_CREATE_RENDER_TEXTURE] = { &NSBInterpreter::CreateRenderTexture, 4 };
    Builtins[MAGIC_DRAW_TRANSITION] = { &NSBInterpreter::DrawTransition, 8 };
    Builtins[MAGIC_CREATE_COLOR] = { &NSBInterpreter::CreateColor, 7 };
    Builtins[MAGIC_LOAD_IMAGE] = { &NSBInterpreter::LoadImage, 2 };
    Builtins[MAGIC_FADE] = { &NSBInterpreter::Fade, 5 };
    Builtins[MAGIC_DELETE] = { &NSBInterpreter::Delete, 1 };
    Builtins[MAGIC_CLEAR_PARAMS] = { &NSBInterpreter::ClearParams, 0 };
    Builtins[MAGIC_SET_LOOP] = { &NSBInterpreter::SetLoop, 2 };
    Builtins[MAGIC_SET_VOLUME] = { &NSBInterpreter::SetVolume, 4 };
    Builtins[MAGIC_SET_LOOP_POINT] = { &NSBInterpreter::SetLoopPoint, 3 };
    Builtins[MAGIC_CREATE_SOUND] = { &NSBInterpreter::CreateSound, 3 };
    Builtins[MAGIC_REMAIN_TIME] = { &NSBInterpreter::RemainTime, 1 };
    Builtins[MAGIC_CREATE_MOVIE] = { &NSBInterpreter::CreateMovie, 7 };
    Builtins[MAGIC_DURATION_TIME] = { &NSBInterpreter::DurationTime, 1 };
    Builtins[MAGIC_SET_FREQUENCY] = { &NSBInterpreter::SetFrequency, 4 };
    Builtins[MAGIC_SET_PAN] = { &NSBInterpreter::SetPan, 4 };
    Builtins[MAGIC_SET_ALIAS] = { &NSBInterpreter::SetAlias, 2 };
    Builtins[MAGIC_CREATE_NAME] = { &NSBInterpreter::CreateName, 1 };
    Builtins[MAGIC_CREATE_WINDOW] = { &NSBInterpreter::CreateWindow, 7 };
    Builtins[MAGIC_CREATE_CHOICE] = { &NSBInterpreter::CreateChoice, NSB_VARARGS };
    Builtins[MAGIC_CASE] = { &NSBInterpreter::Case, 0 };
    Builtins[MAGIC_CASE_END] = { &NSBInterpreter::CaseEnd, 0 };
    Builtins[MAGIC_SET_NEXT_FOCUS] = { &NSBInterpreter::SetNextFocus, 3 };
    Builtins[MAGIC_PASSAGE_TIME] = { &NSBInterpreter::PassageTime, 1 };
    Builtins[MAGIC_PARSE_TEXT] = { &NSBInterpreter::ParseText, 0 };
    Builtins[MAGIC_LOAD_TEXT] = { &NSBInterpreter::LoadText, 7 };
    Builtins[MAGIC_WAIT_TEXT] = { &NSBInterpreter::WaitText, 2 };
    Builtins[MAGIC_LOCK_VIDEO] = { &NSBInterpreter::LockVideo, 1 };
    Builtins[MAGIC_SAVE] = { &NSBInterpreter::Save, 1 };
    Builtins[MAGIC_DELETE_SAVE_FILE] = { &NSBInterpreter::DeleteSaveFile, 1};
    Builtins[MAGIC_CONQUEST] = { &NSBInterpreter::Conquest, 3 };
    Builtins[MAGIC_CLEAR_SCORE] = { &NSBInterpreter::ClearScore, 1 };
    Builtins[MAGIC_CLEAR_BACKLOG] = { &NSBInterpreter::ClearBacklog, 0 };
    Builtins[MAGIC_SET_FONT] = { &NSBInterpreter::SetFont, 6 };
    Builtins[MAGIC_SET_SHORTCUT] = { &NSBInterpreter::SetShortcut, 2 };
    Builtins[MAGIC_CREATE_CLIP_TEXTURE] = { &NSBInterpreter::CreateClipTexture, 9 };
    Builtins[MAGIC_EXIST_SAVE] = { &NSBInterpreter::ExistSave, 1 };
    Builtins[MAGIC_WAIT_ACTION] = { &NSBInterpreter::WaitAction, NSB_VARARGS };
    Builtins[MAGIC_LOAD] = { &NSBInterpreter::Load, NSB_VARARGS };
    Builtins[MAGIC_SET_BACKLOG] = { &NSBInterpreter::SetBacklog, 3 };
    Builtins[MAGIC_CREATE_TEXT] = { &NSBInterpreter::CreateText, 7 };
    Builtins[MAGIC_AT_EXPRESSION] = { &NSBInterpreter::AtExpression, 1 };
    Builtins[MAGIC_RANDOM] = { &NSBInterpreter::Random, 1 };
    Builtins[MAGIC_CREATE_EFFECT] = { &NSBInterpreter::CreateEffect, 7 };
    Builtins[MAGIC_SET_TONE] = { &NSBInterpreter::SetTone, 2 };
    Builtins[MAGIC_DATE_TIME] = { &NSBInterpreter::DateTime, 6};
    Builtins[MAGIC_SHAKE] = { &NSBInterpreter::Shake, 9};
    Builtins[MAGIC_MOVIE_PLAY] = { &NSBInterpreter::MoviePlay, 2};
    Builtins[MAGIC_SET_STREAM] = { &NSBInterpreter::SetStream, 2};
    Builtins[MAGIC_WAIT_PLAY] = { &NSBInterpreter::WaitPlay, 2};
    Builtins[MAGIC_WAIT_FADE] = { &NSBInterpreter::WaitFade, 2};
    Builtins[MAGIC_SOUND_AMPLITUDE] = { &NSBInterpreter::SoundAmplitude, 2};
    Builtins[MAGIC_ROTATE] = { &NSBInterpreter::Rotate, 7};
    Builtins[MAGIC_MESSAGE] = { &NSBInterpreter::Message, 4};
    Builtins[MAGIC_INTEGER] = { &NSBInterpreter::Integer, 1};
    Builtins[MAGIC_CREATE_SCROLLBAR] = { &NSBInterpreter::CreateScrollbar, 9};
    Builtins[MAGIC_SET_SCROLLBAR_VALUE] = { &NSBInterpreter::SetScrollbarValue, 2};
    Builtins[MAGIC_SET_SCROLLBAR_WHEEL_AREA] = { &NSBInterpreter::SetScrollbarWheelArea, 5};
    Builtins[MAGIC_SCROLLBAR_VALUE] = { &NSBInterpreter::ScrollbarValue, 1};
    Builtins[MAGIC_CREATE_STENCIL] = { &NSBInterpreter::CreateStencil, 7};
    Builtins[MAGIC_CREATE_MASK] = { &NSBInterpreter::CreateMask, 6};

    pContext = new NSBContext("__main__");
    pContext->Start();
    Threads.push_back(pContext);
}

NSBInterpreter::~NSBInterpreter()
{
    if (pDebuggerThread)
        pDebuggerThread->join();

    delete pDebuggerThread;
    for (NSBContext* pContext : Threads)
        if (pContext->GetName() == "__main__" || pContext->GetName() == "UNK")
            delete pContext;
}

void NSBInterpreter::ExecuteLocalScript(const string& Filename)
{
    ScriptFile* pScript = new ScriptFile(Filename, ScriptFile::NSS);
    for (const string& i : pScript->GetIncludes())
        sResourceMgr->GetScriptFile(i);
    pContext->Call(pScript, "chapter.main");
}

void NSBInterpreter::ExecuteScript(const string& Filename)
{
    LoadGlobals();
    pBootScript = sResourceMgr->GetScriptFile(Filename);
    if (!pBootScript)
        return;
    pContext->Call(pBootScript, "chapter.main");
}

void NSBInterpreter::OnBackground()
{
    SaveGlobals();
}

bool NSBInterpreter::HorizontalText()
{
    return GetBool("#SYSTEM_text_horizontal") || getenv("NP_TEXT_H");
}

bool NSBInterpreter::IsFastForwarding()
{
    for (NSBContext* pThread : Threads)
        if (pThread->FastForward)
            return true;
    return false;
}

void NSBInterpreter::ExecuteScriptThread(const string& Filename)
{
    NSBContext* pThread = new NSBContext("UNK");
    AddThread(pThread);
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(Filename))
        pThread->Call(pScript, "chapter.main");
}

void NSBInterpreter::Run(int NumCommands)
{
    for (int i = 0; i < NumCommands; ++i)
        RunCommand();
}

void NSBInterpreter::RunCommand()
{
    if (Threads.empty())
        Exit();

    if (!RunInterpreter)
        return;

    ThreadsModified = false;
    for (auto i = Threads.begin(); i != Threads.end(); ++i)
    {
        pContext = *i;
        pContext->TryWake();

        while (pContext->IsActive() && !pContext->IsStarving() && !pContext->IsSleeping() && pContext->Advance() != MAGIC_CLEAR_PARAMS)
        {
            if (pContext->GetName() == "__main__")
                DebuggerTick();

            if (pContext->GetMagic() < Builtins.size())
                 Call(pContext->GetMagic());
        }

        ClearParams();
        if (pContext->IsStarving())
        {
            if (getenv("NP_THR_DBG"))
            {
                fprintf(stderr, "[thr] starve ctx=%p name=%s threads=%zu\n",
                        (void*)pContext, pContext->GetName().c_str(), Threads.size());
            }
            ObjectHolder.Delete(pContext->GetName());
            RemoveThread(pContext);
        }

        if (ThreadsModified)
            break;
    }
}

void NSBInterpreter::Update(uint32_t Diff)
{
    for (NSBContext* pContext : Threads)
        pContext->Update(Diff);
}

void NSBInterpreter::PushEvent(const SDL_Event& Event)
{
    for (auto i = EventOrder.rbegin(); i != EventOrder.rend(); ++i)
    {
        auto Queue = EventQueues.find(*i);
        if (Queue != EventQueues.end())
        {
            Queue->second.push(Event);
            if (getenv("NP_EVENT_DBG"))
                fprintf(stderr, "[event] route type=%x target=%p name=%s script=%s selectors=%zu\n",
                        Event.type, (void*)*i, (*i)->GetName().c_str(),
                        (*i)->GetScriptName().c_str(), EventQueues.size());
            return;
        }
    }
}

void NSBInterpreter::HandleEvent(const SDL_Event& Event)
{
    /*
      TODO:
      $SYSTEM_mousewheel_down
      $SYSTEM_mousewheel_up
      _onetime
    */
    switch (Event.type)
    {
    case SDL_MOUSEBUTTONDOWN:
    {
        ProcessButton(Event.button.button, "true");
        const bool ModalMenuActive =
            GetBool("$SYSTEM_menu_save_enable") ||
            GetBool("$SYSTEM_menu_load_enable") ||
            GetBool("$SYSTEM_menu_config_enable") ||
            GetBool("$SYSTEM_backlog_enable") ||
            GetBool("$SYSTEM_menu_close_enable") ||
            GetBool("$SYSTEM_reset_enable");
        for (auto pContext : Threads)
            pContext->OnClick(!ModalMenuActive);
        if (ModalMenuActive && getenv("NP_EVENT_DBG"))
            fprintf(stderr, "[event] text advancement suppressed by modal menu\n");
        break;
    }
    case SDL_MOUSEBUTTONUP:
        ProcessButton(Event.button.button, "false");
        break;
    case SDL_KEYDOWN:
        if (Event.key.keysym.sym == SDLK_F1)
            SkipHack = !SkipHack;

        ProcessKey(Event.key.keysym.sym, "true");
        for (NSBShortcut& Shortcut : Shortcuts)
            if (Shortcut.Key == Event.key.keysym.sym)
                CallScriptThread(Shortcut.Script, "chapter.main");
        break;
    case SDL_KEYUP:
        ProcessKey(Event.key.keysym.sym, "false");
        break;
    }
}

void NSBInterpreter::ProcessButton(int button, const string& Val)
{
    if (button == SDL_BUTTON_LEFT)
        SetString("$SYSTEM_l_button_down", Val);
    else if (button == SDL_BUTTON_RIGHT)
        SetString("$SYSTEM_r_button_down", Val);
}

void NSBInterpreter::ProcessKey(int Key, const string& Val)
{
    static const map<int, string> keydown =
    {
        { SDLK_RCTRL, "$SYSTEM_keydown_ctrl" },
        { SDLK_KP_ENTER, "$SYSTEM_keydown_enter" },
        { SDLK_RETURN, "$SYSTEM_keydown_enter" },
        { SDLK_END, "$SYSTEM_keydown_end" },
        { SDLK_ESCAPE, "$SYSTEM_keydown_esc" },
        { SDLK_LEFT, "$SYSTEM_keydown_left" },
        { SDLK_RIGHT, "$SYSTEM_keydown_right" },
        { SDLK_DOWN, "$SYSTEM_keydown_up" },
        { SDLK_UP, "$SYSTEM_keydown_down" },
        { SDLK_ESCAPE, "$SYSTEM_keydown_esc" },
        { SDLK_HOME, "$SYSTEM_keydown_home" },
        { SDLK_PAGEDOWN, "$SYSTEM_keydown_pagedown" },
        { SDLK_PAGEUP, "$SYSTEM_keydown_pageup" },
        { SDLK_RSHIFT, "$SYSTEM_keydown_shift" },
        { SDLK_SPACE, "$SYSTEM_keydown_space" },
        { SDLK_TAB, "$SYSTEM_keydown_tab" }
    };
    if (Key >= SDLK_a && Key <= SDLK_z)
    {
        char c = 'a' + Key - SDLK_a;
        string Identifier = "$SYSTEM_keydown_" + string(1, c);
        SetString(Identifier, Val);
    }
    auto it = keydown.find(Key);
    if (it != keydown.end())
        SetString(it->second, Val);
}

void NSBInterpreter::FunctionDeclaration()
{
    Params.Begin(pContext->GetNumParams() - 1);
    for (int i = 1; i < pContext->GetNumParams(); ++i)
        Assign_(i);
}

void NSBInterpreter::CallFunction()
{
    string Symbol = pContext->GetParam(0);
    bool DropLast = false;
    bool AppendEmpty = false;
    string Target;
    if (HorizontalText() && GetHorizontalRedirect(Symbol, Target, DropLast, AppendEmpty) &&
        ResolveFunction(pContext, "function." + Target))
    {
        if (getenv("NP_FUNC_DBG"))
            fprintf(stderr, "[function-redirect] %s -> %s\n", Symbol.c_str(), Target.c_str());
        if (DropLast)
            Params.DropTop();
        if (AppendEmpty)
            Params.PushString("");
        Symbol = Target;
    }

    if (Symbol.compare(0, 7, "SetText") == 0)
    {
        bool Horizontal = !Symbol.empty() && Symbol.back() == 'H';
        if (HasLastTextOrientation && LastTextHorizontal != Horizontal)
        {
            SetTextOrientation(Horizontal);
            TextOrientationTransitionPending = true;
        }
        HasLastTextOrientation = true;
        LastTextHorizontal = Horizontal;
    }

    if (Symbol.compare(0, 9, "TypeBegin") == 0)
        LastTextWindowSymbol = Symbol;

    if (TextOrientationTransitionPending && Symbol.compare(0, 9, "TypeBegin") == 0)
    {
        bool Once = Symbol.compare(0, 13, "TypeBeginOnce") == 0;
        if (!Once)
        {
            string Box;
            if (IsHorizontalTypeBegin(Symbol))
                Box = GetBool("$FwH_text_Window") ? "@boxTH" : "@boxH";
            else
            {
                char Side = GetTypeBeginSide(Symbol);
                string Prefix(1, Side == 'R' || Side == 'L' ? Side : 'C');
                Box = GetBool("$Fw" + Prefix + "_text_Window") ?
                    "@box" + Prefix + "T" : "@box" + Prefix + "C";
            }
            ObjectHolder.Execute(Box, [](Object** ppObject)
            {
                if (Texture* pTexture = dynamic_cast<Texture*>(*ppObject))
                    pTexture->Fade(0, 1000);
            });
        }
        TextOrientationTransitionPending = false;
    }
    if (ReplayTargetRestorePending && Symbol.compare(0, 9, "TypeBegin") == 0)
        RestoreTextWindow(Symbol, true);
    else if (Symbol.compare(0, 9, "TypeBegin") == 0 &&
             Symbol.size() >= 3 && Symbol.compare(Symbol.size() - 3, 3, "IFO") == 0)
        RestoreTextWindow(Symbol, false);
    CallFunction_(pContext, Symbol);
}

void NSBInterpreter::CallScene()
{
    CallScriptSymbol("scene.");
}

void NSBInterpreter::CallChapter()
{
    CallScriptSymbol("chapter.");
}

void NSBInterpreter::CmpLogicalAnd()
{
    BoolBinaryOp(logical_and<bool>());
}

void NSBInterpreter::CmpLogicalOr()
{
    BoolBinaryOp(logical_or<bool>());
}

void NSBInterpreter::CmpGE()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(greater_equal<float>());
    else
        IntBinaryOp(greater_equal<int32_t>());
}

void NSBInterpreter::CmpGreater()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(greater<float>());
    else
        IntBinaryOp(greater<int32_t>());
}

void NSBInterpreter::CmpLess()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(less<float>());
    else
        IntBinaryOp(less<int32_t>());
}

void NSBInterpreter::CmpLE()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(less_equal<float>());
    else
        IntBinaryOp(less_equal<int32_t>());
}

void NSBInterpreter::CmpEqual()
{
    Variable* pLhs = PopVar();
    Variable* pRhs = PopVar();

    bool Equal = false;
    if (pLhs->IsInt() && pRhs->IsInt())
        Equal = pLhs->ToInt() == pRhs->ToInt();
    else if (pLhs->IsString() && pRhs->IsString())
        Equal = pLhs->ToString() == pRhs->ToString();
    else if (pLhs->ToFloat() && pRhs->ToFloat())
        Equal = pLhs->ToFloat() == pRhs->ToFloat();

    Variable::Destroy(pLhs);
    Variable::Destroy(pRhs);
    PushVar(Variable::MakeInt(Equal));
}

void NSBInterpreter::CmpNE()
{
    CmpEqual();
    Call(MAGIC_NOT_EXPRESSION);
}

void NSBInterpreter::NotExpression()
{
    PushVar(Variable::MakeInt(!PopBool()));
}

void NSBInterpreter::AddExpression()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(plus<float>());
    else
    {
        Variable* pLhs = PopVar();
        Variable* pRhs = PopVar();
        PushVar(Variable::Add(pLhs, pRhs));
    }
}

void NSBInterpreter::SubExpression()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(minus<float>());
    else
        IntBinaryOp(minus<int32_t>());
}

void NSBInterpreter::MulExpression()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(multiplies<float>());
    else
        IntBinaryOp(multiplies<int32_t>());
}

void NSBInterpreter::DivExpression()
{
    if (Params.Top()->IsFloat() || Params.TTop()->IsFloat())
        FloatBinaryOp(divides<float>());
    else
        IntBinaryOp(divides<int32_t>());
}

void NSBInterpreter::ModExpression()
{
    IntBinaryOp(modulus<int32_t>());
}

void NSBInterpreter::Increment()
{
    if (Params.Top()->Name == "$SW_PHONE_SENDMAILNO")
    {
        Variable* pVar = PopVar();
        int32_t Index = Nsb::ConstantToValue<Nsb::PhoneMail>(pVar->ToString());
        string Constant = Nsb::ValueToConstant<Nsb::PhoneMail>(Index + 1);
        pVar->Set(Constant);
        PushVar(pVar);
    }
    else
    {
        if (getenv("NSB_COUNT_DBG"))
        {
            Variable* v = Params.Top();
            fprintf(stderr, "[INC] %s = %s -> %d\n", v->Name.c_str(), v->ToString().c_str(), v->ToInt()+1);
        }
        IntUnaryOp([](int32_t a) { return ++a; });
    }
}

void NSBInterpreter::Decrement()
{
    IntUnaryOp([](int32_t a) { return --a; });
}

void NSBInterpreter::Literal()
{
    const string& Type = pContext->GetParam(0);
    const string& Val = pContext->GetParam(1);
    if (Type == "STRING")
    {
        if (Variable* pVar = VariableHolder.Read(Val))
            PushVar(pVar);
        else
            PushString(Val);
    }
    else if (Type == "INT")
        PushInt(stoi(Val));
    else if (Type == "FLOAT")
        PushFloat(stof(Val));
}

void NSBInterpreter::Assign()
{
    if (pContext->GetParam(0) == "__array_variable__")
    {
        Params.Begin(1);
        Variable* pVar = PopVar();
        Variable* pLit = PopVar();
        pVar->Set(pLit);
        Variable::Destroy(pLit);
    }
    else
        Assign_(0);
}

void NSBInterpreter::Get()
{
    PushVar(GetVar(pContext->GetParam(0)));
}

void NSBInterpreter::ScopeBegin()
{
}

void NSBInterpreter::ScopeEnd()
{
}

void NSBInterpreter::Return()
{
    pContext->Return();
}

void NSBInterpreter::If()
{
    if (!PopBool())
        Jump();
}

void NSBInterpreter::While()
{
    pContext->PushBreak();
    If();
}

void NSBInterpreter::WhileEnd()
{
    pContext->PopBreak();
}

/*
 * Select an event for next iteration of event loop. (see: Case)
 * Valid events are: Mouse Up/Down/Wheel, Arrow Up/Down/Right/Left
 * */
void NSBInterpreter::Select()
{
    auto Inserted = EventQueues.emplace(pContext, queue<SDL_Event>());
    if (Inserted.second)
        EventOrder.push_back(pContext);
    if (Inserted.second && getenv("NP_EVENT_DBG"))
        fprintf(stderr, "[event] register ctx=%p name=%s script=%s threads=%zu selectors=%zu\n",
                (void*)pContext, pContext->GetName().c_str(),
                pContext->GetScriptName().c_str(), Threads.size(), EventQueues.size());
    pWindow->Select(true);
    if (SelectEvent())
        pContext->PushBreak();
}

void NSBInterpreter::SelectEnd()
{
    SelectEvent();
}

void NSBInterpreter::SelectBreakEnd()
{
    pContext->PopBreak();
    EventQueues.erase(pContext);
    EventOrder.remove(pContext);
    pWindow->Select(!EventQueues.empty());
}

void NSBInterpreter::Break()
{
    pContext->Break();
}

void NSBInterpreter::Jump()
{
    pContext->Jump(pContext->GetParam(0));
}

Variable* NSBInterpreter::PopVar()
{
    return Params.Pop();
}

Texture* NSBInterpreter::PopTexture()
{
    return Get<Texture>(PopString());
}

GLTexture* NSBInterpreter::PopGLTexture()
{
    return Get<GLTexture>(PopString());
}

Playable* NSBInterpreter::PopPlayable()
{
    return Get<Playable>(PopString());
}

Scrollbar* NSBInterpreter::PopScrollbar()
{
    return Get<Scrollbar>(PopString());
}

int32_t NSBInterpreter::PopInt()
{
    Variable* pVar = PopVar();
    int32_t Val = pVar->ToInt();
    Variable::Destroy(pVar);
    return Val;
}

float NSBInterpreter::PopFloat()
{
    Variable* pVar = PopVar();
    float Val = pVar->ToFloat();
    Variable::Destroy(pVar);
    return Val;
}

string NSBInterpreter::PopString()
{
    Variable* pVar = PopVar();
    string Val = pVar->ToString();
    Variable::Destroy(pVar);
    return Val;
}

NSBPosition NSBInterpreter::PopPos()
{
    const int32_t WIDTH = pWindow->WIDTH;
    const int32_t HEIGHT = pWindow->HEIGHT;
    static const size_t SPECIAL_POS_NUM = 19;
    static const PosFunc SpecialPosTable[SPECIAL_POS_NUM] =
    {
        [WIDTH] (int32_t x) { return WIDTH; }, // OutRight
        [] (int32_t x) { return -x; }, // OutLeft
        [] (int32_t y) { return -y; }, // OutTop
        [HEIGHT] (int32_t y) { return HEIGHT; }, // OutBottom

        [WIDTH] (int32_t x) { return WIDTH - x; }, // InRight
        [] (int32_t x) { return 0; }, // InLeft
        [] (int32_t y) { return 0; }, // InTop
        [HEIGHT] (int32_t y) { return HEIGHT - y; }, // InBottom

        [WIDTH] (int32_t x) { return WIDTH - (x / 2); }, // OnRight
        [] (int32_t x) { return -(x / 2); }, // OnLeft
        [] (int32_t y) { return -(y / 2); }, // OnTop
        [HEIGHT] (int32_t y) { return HEIGHT - (y / 2); }, // OnBottom

        [] (int32_t x) { return x; }, // Right
        [] (int32_t x) { return 0; }, // Left
        [] (int32_t y) { return 0; }, // Top
        [] (int32_t y) { return y; }, // Bottom

        [WIDTH] (int32_t x) { return (WIDTH - x) / 2; }, // Center
        [HEIGHT] (int32_t y) { return (HEIGHT - y) / 2; }, // Middle
        [] (int32_t xy) { return numeric_limits<int32_t>::max(); } // Auto
    };
    static const string SpecialPos[SPECIAL_POS_NUM] =
    {
        "outright", "outleft", "outtop", "outbottom",
        "inright", "inleft", "intop", "inbottom",
        "onright", "onleft", "ontop", "onbottom",
        "right", "left", "top", "bottom",
        "center", "middle", "auto"
    };

    NSBPosition Position;
    Variable* pVar = PopVar();
    Position.Relative = pVar->Relative;
    if (pVar->IsInt())
    {
        int32_t Val = pVar->ToInt();
        Position.Func = [Val] (int32_t) { return Val; };
    }
    else
    {
        string Str = pVar->ToString();
        transform(Str.begin(), Str.end(), Str.begin(), ::tolower);
        size_t i = -1;
        while (++i < SPECIAL_POS_NUM)
            if (Str == SpecialPos[i])
                Position.Func = SpecialPosTable[i];
    }
    Variable::Destroy(pVar);
    return Position;
}

NSBPosition NSBInterpreter::PopRelative()
{
    NSBPosition Position;
    Variable* pVar = PopVar();
    Position.Relative = pVar->Relative;
    int32_t Val = pVar->ToInt();
    Position.Func = [Val] (int32_t) { return Val; };
    Variable::Destroy(pVar);
    return Position;
}

uint32_t NSBInterpreter::PopColor()
{
    uint32_t Color = 0;
    Variable* pVar = PopVar();
    if (pVar->IsString())
    {
        string Str = boost::algorithm::to_lower_copy(pVar->ToString());
        if (Nsb::IsValidConstant<Nsb::Color>(Str))
            Color = Nsb::ConstantToValue<Nsb::Color>(Str);
        else
        {
            size_t i = Str.find_first_of("0123456789abcdef");
            if (i != Str.npos && Str.size() - i >= 6)
                Color = stoi(Str.substr(i, 6), nullptr, 16) | (0xFF << 24);
        }
    }
    else
        Color = stoi(to_string(pVar->ToInt()), nullptr, 16) | (0xFF << 24);

    Variable::Destroy(pVar);
    return Color;
}

int32_t NSBInterpreter::PopRequest()
{
    return Nsb::ConstantToValue<Nsb::Request>(PopString());
}

int32_t NSBInterpreter::PopTone()
{
    return Nsb::ConstantToValue<Nsb::Tone>(PopString());
}

int32_t NSBInterpreter::PopEffect()
{
    return Nsb::ConstantToValue<Nsb::Effect>(PopString());
}

int32_t NSBInterpreter::PopShade()
{
    return Nsb::ConstantToValue<Nsb::Shade>(PopString());
}

int32_t NSBInterpreter::PopTempo()
{
    return Nsb::ConstantToValue<Nsb::Tempo>(PopString());
}

bool NSBInterpreter::PopBool()
{
    Variable* pVar = PopVar();
    bool ret = ToBool(pVar);
    Variable::Destroy(pVar);
    return ret;
}

string NSBInterpreter::PopSave()
{
    boost::format Fmt("%s/%04d.npf");
    Fmt % GetString("#SYSTEM_save_path");
    Fmt % PopInt();
    return Fmt.str();
}

void NSBInterpreter::PushFloat(float Float)
{
    PushVar(Variable::MakeFloat(Float));
}

void NSBInterpreter::PushInt(int32_t Int)
{
    PushVar(Variable::MakeInt(Int));
}

void NSBInterpreter::PushString(const string& Str)
{
    PushVar(Variable::MakeString(Str));
}

void NSBInterpreter::PushVar(Variable* pVar)
{
    Params.Push(pVar);
}

void NSBInterpreter::Assign_(int Index)
{
    SetVar(pContext->GetParam(Index), PopVar());
}

void NSBInterpreter::IntUnaryOp(function<int32_t(int32_t)> Func)
{
    PushVar(PopVar()->IntUnaryOp(Func));
}

void NSBInterpreter::IntBinaryOp(function<int32_t(int32_t, int32_t)> Func)
{
    int32_t lhs = PopInt();
    int32_t rhs = PopInt();
    PushInt(Func(lhs, rhs));
}

void NSBInterpreter::FloatBinaryOp(function<float(float, float)> Func)
{
    float lhs = PopFloat();
    float rhs = PopFloat();
    PushFloat(Func(lhs, rhs));
}

void NSBInterpreter::BoolBinaryOp(function<bool(bool, bool)> Func)
{
    bool lhs = PopBool();
    bool rhs = PopBool();
    PushInt(Func(lhs, rhs));
}

void NSBInterpreter::CallFunction_(NSBContext* pThread, const string& Symbol)
{
    if (!pThread->Call(pContext->GetScript(), string("function.") + Symbol))
    {
        if (Symbol == "Platform")
        {
            PushInt(100);
            return;
        }

        // Muramasa invokes engine-side helpers through call_function; the
        // compiler emitted no magic for them, so script lookup fails.
        static const std::set<string> EngineCalls =
        {
            "DrawEffect", "SetBlur", "MoveCamera", "SetFov", "CreateCube",
            "BezierMove", "TokutenPatch", "CreateStencil", "CreateMask",
            "LockVideo", "Rot", "Rosso", "DrawScreenTexture", "SendModule"
        };
        if (EngineCalls.count(Symbol))
        {
            if (getenv("NP_FUNC_DBG"))
                NSB_ERROR("Skipped engine function", Symbol);
            return;
        }
        NSB_ERROR("Failed to call function", Symbol);
    }
}

void NSBInterpreter::CallScriptSymbol(const string& Prefix)
{
    string ScriptName = GetString(pContext->GetParam(0)), Symbol;
    size_t i = ScriptName.find("->");
    if (i != string::npos)
    {
        Symbol = ScriptName.substr(i + 2);
        ScriptName.erase(i);
    }
    else
        Symbol = "main";
    CallScript(ScriptName == "@" ? pContext->GetScriptName() : ScriptName, Prefix + Symbol);
}

void NSBInterpreter::CallScript(const string& Filename, const string& Symbol)
{
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(Filename))
        pContext->Call(pScript, Symbol);
}

void NSBInterpreter::CallScriptThread(const string& Filename, const string& Symbol)
{
    NSBContext* pThread = new NSBContext("UNK");
    AddThread(pThread);
    if (ScriptFile* pScript = sResourceMgr->GetScriptFile(Filename))
        pThread->Call(pScript, Symbol);
}

void NSBInterpreter::Call(uint16_t Magic)
{
    static FILE* tr = getenv("NSB_TRACE") ? fopen("/tmp/nsb_exec.log", "a") : nullptr;
    if (tr)
    {
        fprintf(tr, "M%04x %s(%p):%u %s\n", Magic,
                pContext->GetScript() ? pContext->GetScript()->GetName().c_str() : "?",
                (void*)pContext,
                pContext->GetLineNumber(),
                Builtins[Magic].Func ? "" : "(unimpl)");
        fflush(tr);
    }
    uint8_t NumParams = Builtins[Magic].NumParams;
    Params.Begin(NumParams == NSB_VARARGS ? pContext->GetNumParams() : NumParams);

    if (Builtins[Magic].Func)
        (this->*Builtins[Magic].Func)();
}

bool NSBInterpreter::SelectEvent()
{
    auto i = EventQueues.find(pContext);
    if (i != EventQueues.end() && !i->second.empty())
    {
        Event = i->second.front();
        i->second.pop();
        return true;
    }
    else
    {
        pContext->FastForward = false;
        pContext->Rewind();
        pContext->Wait(20);
        return false;
    }
}

void NSBInterpreter::AddThread(NSBContext* pThread)
{
    pThread->Start();
    Threads.push_back(pThread);
    ThreadsModified = true;
}

void NSBInterpreter::RemoveThread(NSBContext* pThread)
{
    Threads.remove(pThread);
    EventQueues.erase(pThread);
    EventOrder.remove(pThread);
    pWindow->Select(!EventQueues.empty());
    ThreadsModified = true;
}

void NSBInterpreter::RemoveThreads(ObjectHolder_t* pHolder)
{
    pHolder->ForEachObject([this](Object* pObject)
    {
        if (NSBContext* pThread = dynamic_cast<NSBContext*>(pObject))
            RemoveThread(pThread);
        else if (pObject)
            RemoveThreads(pObject);
    });
}

int32_t NSBInterpreter::GetInt(const string& Name)
{
    return GetVar(Name)->ToInt();
}

bool NSBInterpreter::GetBool(const string& Name)
{
    return ToBool(GetVar(Name));
}

string NSBInterpreter::GetString(const string& Name)
{
    if (Name[0] != '$' && Name[0] != '#')
        return Name;
    return GetVar(Name)->ToString();
}

bool NSBInterpreter::ToBool(Variable* pVar)
{
    if (pVar->IsString())
    {
        int32_t Val = Nsb::ConstantToValue<Nsb::Boolean>(pVar->ToString());
        if (Val != -1)
            return static_cast<bool>(Val);
    }
    return static_cast<bool>(pVar->ToInt());
}

Variable* NSBInterpreter::GetVar(const string& Name)
{
    if (Variable* pVar = VariableHolder.Read(Name))
        return pVar;

    Variable* pVar = Variable::MakeNull(Name);
    VariableHolder.Write(Name, pVar);
    return pVar;
}

Object* NSBInterpreter::GetObject(const string& Name)
{
    return ObjectHolder.Read(Name);
}

void NSBInterpreter::OnVariableChanged(const string& Name)
{
    if (Name == "#SYSTEM_window_full")
        pWindow->SetFullscreen(GetBool("#SYSTEM_window_full") ? SDL_WINDOW_FULLSCREEN : 0);
    else if (Name == "#SYSTEM_text_horizontal")
    {
        if (getenv("NP_TEX_DBG"))
            fprintf(stderr, "[tex-mode] horizontal=%d\n", HorizontalText());
    }

    if (IsMenuVisibilityVariable(Name))
        UpdateDialogueTextVisibility();
    if (Name == "$SYSTEM_menu_config_enable" && !GetBool("$SYSTEM_menu_config_enable"))
        SaveGlobals();
}

void NSBInterpreter::SetTextOrientation(bool Horizontal)
{
    if (getenv("NP_TEX_DBG"))
        fprintf(stderr, "[tex-mode] orientation transition horizontal=%d\n", Horizontal);

    auto Fade = [this](const string& Handle)
    {
        ObjectHolder.Execute(Handle, [](Object** ppObject)
        {
            if (Texture* pTexture = dynamic_cast<Texture*>(*ppObject))
                pTexture->Fade(0, 0);
        });
    };

    if (Horizontal)
    {
        for (const string Handle : {"@boxCC", "@boxCT", "@boxRR", "@boxRT", "@boxLL", "@boxLT",
                                    "@FwC*", "@FwR*", "@FwL*"})
            Fade(Handle);
        for (const string Name : {"$FwC_text_Window", "$FwC_text_Window_begin",
                                  "$FwR_text_Window", "$FwR_text_Window_begin",
                                  "$FwL_text_Window", "$FwL_text_Window_begin"})
            SetString(Name, "false");
    }
    else
    {
        for (const string Handle : {"@boxH", "@boxTH", "@FwH*"})
            Fade(Handle);
        SetString("$FwH_text_Window", "false");
        SetString("$FwH_text_Window_begin", "false");
    }
}

void NSBInterpreter::RestoreTextWindow(const string& Symbol, bool Replay)
{
    if (Replay)
        ReplayTargetRestorePending = false;
    if (Symbol.compare(0, 13, "TypeBeginOnce") == 0)
        return;

    auto Fade = [this](const string& Handle, int32_t Opacity)
    {
        ObjectHolder.Execute(Handle, [Opacity](Object** ppObject)
        {
            if (Texture* pTexture = dynamic_cast<Texture*>(*ppObject))
                pTexture->Fade(0, Opacity);
        });
    };

    bool HasFace;
    string Box;
    string Face;
    if (IsHorizontalTypeBegin(Symbol))
    {
        HasFace = GetBool("$FwH_text_Window");
        Box = HasFace ? "@boxTH" : "@boxH";
        Face = "@FwH*";
        for (const char* Handle : {"@boxCC", "@boxCT", "@boxRR", "@boxRT",
                                   "@boxLL", "@boxLT", "@FwC*", "@FwR*", "@FwL*"})
            Fade(Handle, 0);
        Fade(Face, 0);
        Fade(HasFace ? "@boxH" : "@boxTH", 0);
    }
    else
    {
        char Side = GetTypeBeginSide(Symbol);
        string Prefix(1, Side == 'R' || Side == 'L' ? Side : 'C');
        HasFace = GetBool("$Fw" + Prefix + "_text_Window");
        Box = "@box" + Prefix + (HasFace ? "T" : "C");
        Face = "@Fw" + Prefix + "*";
        for (const char* Handle : {"@boxH", "@boxTH", "@FwH*"})
            Fade(Handle, 0);
        for (const string Handle : {"@boxCC", "@boxCT", "@boxRR", "@boxRT",
                                    "@boxLL", "@boxLT", "@FwC*", "@FwR*", "@FwL*"})
            Fade(Handle, Handle == Box || (HasFace && Handle == Face) ? 1000 : 0);
    }

    if (Symbol.find('H') != string::npos)
        Fade(Box, 1000);
    if (HasFace)
        Fade(Face, 1000);

    if (getenv("NP_TEX_DBG"))
    {
        if (Replay)
            fprintf(stderr, "[load] restore text window symbol=%s box=%s face=%s\n",
                    Symbol.c_str(), Box.c_str(), HasFace ? Face.c_str() : "none");
        else
            fprintf(stderr, "[text-window] menu restore symbol=%s box=%s face=%s\n",
                    Symbol.c_str(), Box.c_str(), HasFace ? Face.c_str() : "none");
    }
}

void NSBInterpreter::UpdateDialogueTextVisibility()
{
    bool MenuOpen = GetBool("$SYSTEM_menu_enable") ||
                    GetBool("$SYSTEM_menu_save_enable") ||
                    GetBool("$SYSTEM_menu_load_enable") ||
                    GetBool("$SYSTEM_menu_config_enable") ||
                    GetBool("$SYSTEM_backlog_enable") ||
                    GetBool("$SYSTEM_backselect_enable") ||
                    GetBool("$SYSTEM_menu_close_enable") ||
                    GetBool("$SYSTEM_reset_enable");
    bool MenuOpened = !DialogueMenuWasOpen && MenuOpen;
    bool MenuClosed = DialogueMenuWasOpen && !MenuOpen;

    if (MenuOpened)
    {
        MenuTextureOpacities.clear();
        function<void(ObjectHolder_t*)> CaptureTextures = [&](ObjectHolder_t* Holder)
        {
            Holder->ForEachObject([&](Object* pObject)
            {
                if (Texture* pTexture = dynamic_cast<Texture*>(pObject))
                    if (!pTexture->DebugName.empty() &&
                        !IsDialogueWindowTexture(pTexture->DebugName))
                        MenuTextureOpacities[pTexture->DebugName] = pTexture->GetOpacity();
                if (pObject)
                    CaptureTextures(pObject);
            });
        };
        CaptureTextures(&ObjectHolder);
        MenuTextureSnapshotCaptured = true;
        if (getenv("NP_TEX_DBG"))
            fprintf(stderr, "[text-window] menu snapshot textures=%zu\n", MenuTextureOpacities.size());
    }

    DialogueMenuWasOpen = MenuOpen;

    if (MenuClosed)
    {
        if (MenuTextureSnapshotCaptured)
            for (const auto& Entry : MenuTextureOpacities)
                if (Texture* pTexture = dynamic_cast<Texture*>(ObjectHolder.Read(Entry.first)))
                    pTexture->Fade(0, Entry.second);
        if (getenv("NP_TEX_DBG"))
            fprintf(stderr, "[text-window] menu restore symbol=%s textures=%zu\n",
                    LastTextWindowSymbol.c_str(), MenuTextureOpacities.size());
        MenuTextureOpacities.clear();
        MenuTextureSnapshotCaptured = false;
    }

    if (Variable* pHandle = VariableHolder.Read("$SYSTEM_present_text"))
    {
        string Handle = pHandle->ToString();
        if (!Handle.empty())
            if (Text* pText = Get<Text>(Handle))
            {
                if (getenv("NP_TEX_DBG"))
                    fprintf(stderr, "[text-layer] handle=%s priority=%d menu=%d\n",
                            Handle.c_str(), pText->GetPriority(), MenuOpen);
                if (MenuOpen)
                    pText->Fade(0, 0);
                else
                    pText->Fade(0, 1000);
            }
    }

    if (MenuClosed && !LastTextWindowSymbol.empty())
        RestoreTextWindow(LastTextWindowSymbol, false);
}

void NSBInterpreter::SetVar(const string& Name, Variable* pVar)
{
    if (Name == "$GameName" && pContext && pContext->FastForward &&
        !ReplayGameName.empty() && pVar->ToString() != ReplayGameName)
        pContext->FastForward = false;
    GetVar(Name)->Set(pVar);
    OnVariableChanged(Name);
    Variable::Destroy(pVar);
}

void NSBInterpreter::SetInt(const string& Name, int32_t Val)
{
    SetVar(Name, Variable::MakeInt(Val));
}

void NSBInterpreter::SetString(const string& Name, const string& Val)
{
    SetVar(Name, Variable::MakeString(Val));
}

void NSBInterpreter::AddAssign()
{
    Variable* pVar = GetVar(pContext->GetParam(0));
    Variable* pTemp = Variable::Add(pVar, PopVar());
    pVar->Set(pTemp);
    Variable::Destroy(pTemp);
    OnVariableChanged(pContext->GetParam(0));
}

void NSBInterpreter::SubAssign()
{
    Variable* pVar = GetVar(pContext->GetParam(0));
    Variable* pTemp = Variable::MakeInt(pVar->ToInt() - PopInt());
    pVar->Set(pTemp);
    Variable::Destroy(pTemp);
    OnVariableChanged(pContext->GetParam(0));
}

void NSBInterpreter::ModAssign()
{
    Variable* pVar = GetVar(pContext->GetParam(0));
    Variable* pTemp = Variable::MakeInt(pVar->ToInt() % PopInt());
    pVar->Set(pTemp);
    Variable::Destroy(pTemp);
    OnVariableChanged(pContext->GetParam(0));
}

void NSBInterpreter::WriteFile()
{
    string Filename = PopString();
    string Data = NpaFile::FromUtf8(PopString());
    uint32_t Size = Data.size();
    fs::WriteFileDirectory(Filename, NpaFile::Encrypt(&Data[0], Size), Size);
}

void NSBInterpreter::ReadFile()
{
    string Filename = PopString();
    uint32_t Size;
    char* pData = fs::ReadFile(Filename, Size);
    if (!pData)
    {
        if (getenv("NP_READFILE_DBG"))
            fprintf(stderr, "[script] ReadFile missing %s\n", Filename.c_str());
        PushString("");
        return;
    }
    NpaFile::Decrypt(pData, Size);
    PushString(NpaFile::ToUtf8(pData, Size));
    delete[] pData;
}

void NSBInterpreter::CreateTexture()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();
    string Source = PopString();

    Texture* pTexture = new Texture;
    if (Source == "VIDEO")
        ;
    else if (Source == "SCREEN")
        pTexture->CreateFromScreen(pWindow);
    else if (Source.size() < 4 || Source[Source.size() - 4] != '.')
    {
        if (GLTexture* pSource = Get<GLTexture>(Source))
            pTexture->CreateFromGLTexture(pSource);
        else if (Image* pSource = Get<Image>(Source))
            pTexture->CreateFromImage(pSource);
    }
    else
        pTexture->CreateFromFile(Source);

    pTexture->SetVertex(pTexture->GetWidth() / 2, pTexture->GetHeight() / 2);
    int32_t TargetX = X(pTexture->GetWidth());
    int32_t TargetY = Y(pTexture->GetHeight());
    pTexture->Move(TargetX, TargetY);
    if (Handle.rfind("me", 0) == 0 && Priority < 30000)
        Priority = 30000;
    pTexture->SetPriority(Priority);
    pTexture->DebugName = Handle;

    if (getenv("NP_TEX_DBG"))
        fprintf(stderr, "[tex] CreateTexture %s src=%s prio=%d size=%dx%d pos=%d,%d\n",
                Handle.c_str(), Source.c_str(), Priority,
                pTexture->GetWidth(), pTexture->GetHeight(),
                pTexture->GetMX(), pTexture->GetMY());
    pWindow->AddTexture(pTexture);
    ObjectHolder.Write(Handle, pTexture);
}

void NSBInterpreter::ImageHorizon()
{
    Texture* pTexture = PopTexture();
    PushInt(pTexture ? pTexture->GetWidth() : 0);
}

void NSBInterpreter::ImageVertical()
{
    Texture* pTexture = PopTexture();
    PushInt(pTexture ? pTexture->GetHeight() : 0);
}

void NSBInterpreter::Time()
{
    PushInt(time(0));
}

void NSBInterpreter::StrStr()
{
    string Haystack = PopString();
    string Needle = PopString();
    PushInt(Haystack.find(Needle) + 1);
}

void NSBInterpreter::Exit()
{
    SaveGlobals();
    pWindow->Exit();
}

void NSBInterpreter::CursorPosition()
{
    int32_t X, Y;
    SDL_PumpEvents();
    SDL_GetMouseState(&X, &Y);
    GLSurfaceToGame(X, Y);
    SetInt(PopString(), X);
    SetInt(PopString(), Y);
}

void NSBInterpreter::MoveCursor()
{
    int32_t X = PopInt();
    int32_t Y = PopInt();
    GLGameToSurface(X, Y);
    pWindow->MoveCursor(X, Y);
}

void NSBInterpreter::Position()
{
    Texture* pTexture = PopTexture();
    SetInt(pContext->GetParam(1), pTexture ? pTexture->GetX() : 0);
    SetInt(pContext->GetParam(2), pTexture ? pTexture->GetY() : 0);
}

void NSBInterpreter::Wait()
{
    pContext->Wait(PopInt());
}

void NSBInterpreter::WaitKey()
{
    pContext->WaitKey(pContext->GetNumParams() == 1 ? PopInt() : -1);
}

void NSBInterpreter::NegaExpression()
{
    IntUnaryOp(negate<int32_t>());
}

void NSBInterpreter::System()
{
    static const string OpenStr = "OPEN:";
    string Command = PopString();
    string Parameters = PopString();
    string Directory = PopString();
    if (Command.substr(0, OpenStr.size()) != OpenStr)
        return;

    Command = Command.substr(OpenStr.size());

    if (fork() == 0)
        execlp("/usr/bin/xdg-open", "/usr/bin/xdg-open", Command.c_str(), NULL);
}

void NSBInterpreter::String()
{
    boost::format Fmt(PopString());
    for (int i = 1; i < pContext->GetNumParams(); ++i)
    {
        Variable* pVar = PopVar();
        if (pVar->IsInt())
            Fmt % pVar->ToInt();
        else if (pVar->IsString())
            Fmt % pVar->ToString();
        else if (pVar->IsFloat())
            Fmt % pVar->ToFloat();
        Variable::Destroy(pVar);
    }
    PushString(Fmt.str());
}

void NSBInterpreter::VariableValue()
{
    string Type = PopString();
    string Name = PopString();

    // Check if it's array index. Not sure about this...
    if (Name.size() > 3 && Name[Name.size() - 3] == '[' && Name.back() == ']' && isdigit(Name[Name.size() - 2]))
        PushVar(GetVar(Name.substr(0, Name.size() - 3) + "/" + to_string(Name[Name.size() - 2])));
    else if (pContext->GetNumParams() == 3)
        SetVar(Type + Name, PopVar());
    else if (pContext->GetNumParams() == 2)
        PushVar(GetVar(Type + Name));
    else
        assert(false && "This will trigger when we get new season of Haruhi");
}

void NSBInterpreter::CreateProcess()
{
    string Handle = PopString();
    /*int32_t unk1 = */PopInt();
    /*int32_t unk2 = */PopInt();
    /*int32_t unk3 = */PopInt();
    string Symbol = PopString();

    NSBContext* pThread = new NSBContext(Handle);
    CallFunction_(pThread, Symbol);
    AddThread(pThread);
    if (NSBContext* pExisting = dynamic_cast<NSBContext*>(ObjectHolder.Read(Handle)))
        RemoveThread(pExisting);
    ObjectHolder.Write(Handle, pThread);
}

void NSBInterpreter::Count()
{
    Variable* pArr = PopVar();
    int SCount = count(pArr->Name.begin(), pArr->Name.end(), '/') + 1;
    int32_t Size = 0;
    for (auto& i : VariableHolder.Cache)
    {
        if (i.first.size() <= pArr->Name.size() || i.first[pArr->Name.size()] != '/')
            continue;
        string Prefix = i.first.substr(0, pArr->Name.size());
        int Count = count(i.first.begin(), i.first.end(), '/');
        if (Prefix == pArr->Name && SCount == Count)
            Size++;
    }
    static int dc = 0;
    if (getenv("NSB_COUNT_DBG") && dc++ < 40)
        fprintf(stderr, "[COUNT] '%s' -> %d (holder=%zu)\n", pArr->Name.c_str(), Size, VariableHolder.Cache.size());
    PushInt(Size);
}

void NSBInterpreter::Array()
{
    Variable* pArr = PopVar();
    if (!pArr)
    {
        pArr = Variable::MakeNull(pContext->GetParam(0));
        VariableHolder.Write(pContext->GetParam(0), pArr);
    }
    for (int i = 1; i < pContext->GetNumParams(); ++i)
    {
        string Name = pArr->Name + "/" + to_string(i - 1);
        Variable* pVar = Variable::MakeCopy(PopVar(), Name);
        VariableHolder.Write(Name, pVar);
    }
}

void NSBInterpreter::SubScript()
{
    Variable* pArr = GetVar(pContext->GetParam(0));
    int32_t Depth = stoi(pContext->GetParam(1));
    Params.Begin(Depth);
    while (Depth --> 0)
    {
        Variable* pVar = PopVar();
        int Index = pVar->IsInt() ? pVar->ToInt() : pArr->Assoc[pVar->ToString()];
        pArr = GetVar(pArr->Name + "/" + to_string(Index));
        Variable::Destroy(pVar);
    }
    PushVar(pArr);
}

void NSBInterpreter::AssocArray()
{
    Variable* pArr = PopVar();
    for (int i = 1; i < pContext->GetNumParams(); ++i)
        pArr->Assoc[PopString()] = i - 1;
}

void NSBInterpreter::ModuleFileName()
{
    string Name = pContext->GetScriptName();
    PushString(Name.substr(4, Name.size() - 8)); // Remove nss/ and .nsb
}

void NSBInterpreter::Request()
{
    string Handle = PopString();
    int32_t Request = PopRequest();

    ObjectHolder.Execute(Handle, [this, Request] (Object** ppObject)
    {
        if (Object* pObject = *ppObject)
        {
            Playable* pPlayable = dynamic_cast<Playable*>(pObject);
            if (!pContext->FastForward || !pPlayable || pPlayable->IsBGM)
                pObject->Request(Request);
            else if (Request == Nsb::PLAY || Request == Nsb::RESUME)
                pPlayable->DeferFastForwardPlay();
            else
                pPlayable->ClearDeferredFastForwardPlay();
        }
    });
}

void NSBInterpreter::SetVertex()
{
    Texture* pTexture = PopTexture();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();

    if (pTexture)
        pTexture->SetVertex(X(pTexture->GetWidth(), pTexture->GetOX()), Y(pTexture->GetHeight(), pTexture->GetOY()));
}

void NSBInterpreter::Zoom()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    NSBPosition XScale = PopRelative();
    NSBPosition YScale = PopRelative();
    int32_t Tempo = PopTempo();
    bool Wait = PopBool();

    if (pContext->FastForward)
        Time = 0;
    ObjectHolder.Execute(Handle, [&] (Object** ppObject)
    {
        if (Texture* pTexture = dynamic_cast<Texture*>(*ppObject))
            pTexture->Zoom(Time, XScale(0, pTexture->GetXScale()), YScale(0, pTexture->GetYScale()), Tempo);
    });

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::Move()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();
    int32_t Tempo = PopTempo();
    bool Wait = PopBool();

    ObjectHolder.Execute(Handle, [&] (Object** ppObject)
    {
        if (Texture* pTexture = dynamic_cast<Texture*>(*ppObject))
        {
            int32_t TX = X(pTexture->GetWidth(), pTexture->GetMX());
            int32_t TY = Y(pTexture->GetHeight(), pTexture->GetMY());
            if (pContext->FastForward)
                Time = 0;
            pTexture->Move(TX, TY, Time, Tempo);
        }
    });

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::SetShade()
{
    if (Texture* pTexture = PopTexture())
        pTexture->SetShade(PopShade());
}

void NSBInterpreter::DrawToTexture()
{
    GLTexture* pTexture = PopGLTexture();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    string Filename = PopString();

    if (pTexture)
        pTexture->Draw(X, Y, Filename);
}

void NSBInterpreter::CreateRenderTexture()
{
    string Handle = PopString();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    uint32_t Color = PopColor();

    GLTexture* pTexture = new GLTexture;
    pTexture->CreateFromColor(Width, Height, Color);
    ObjectHolder.Write(Handle, pTexture);
}

void NSBInterpreter::DrawTransition()
{
    Texture* pTexture = PopTexture();
    int32_t Time = PopInt();
    int32_t Start = PopInt();
    int32_t End = PopInt();
    int32_t Boundary = PopInt();
    int32_t Tempo = PopTempo();
    string Filename = PopString();
    bool Wait = PopBool();

    if (pTexture)
        pTexture->DrawTransition(Time, Start, End, Boundary, Tempo, Filename);

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::CreateColor()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    uint32_t Color = PopColor();

    Texture* pTexture = new Texture;
    pTexture->CreateFromColor(Width, Height, Color);
    pTexture->SetVertex(Width / 2, Height / 2);
    pTexture->Move(X(Width), Y(Height));
    pTexture->SetPriority(Priority);
    pTexture->DebugName = Handle;

    pWindow->AddTexture(pTexture);
    ObjectHolder.Write(Handle, pTexture);
}

void NSBInterpreter::LoadImage()
{
    string Handle = PopString();
    string Filename = PopString();

    Image* pImage = new Image;
    if (Filename == "SCREEN")
        pImage->LoadScreen(pWindow);
    else
        pImage->LoadImage(Filename);
    ObjectHolder.Write(Handle, pImage);
}

void NSBInterpreter::Fade()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Opacity = PopInt();
    int32_t Tempo = PopTempo();
    bool Wait = PopBool();

    if (pContext->FastForward)
        Time = 0;
    if (getenv("NP_FADE_DBG"))
        fprintf(stderr, "[fade] %s t=%d op=%d wait=%d\n", Handle.c_str(), Time, Opacity, Wait);

    ObjectHolder.Execute(Handle, [Time, Opacity, Tempo] (Object** ppObject)
    {
        if (Texture* pTexture = dynamic_cast<Texture*>(*ppObject))
            pTexture->Fade(Time, Opacity, Tempo);
    });

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::Delete()
{
    string Handle = PopString();
    if (getenv("NP_TEX_DBG"))
        fprintf(stderr, "[tex] Delete %s\n", Handle.c_str());

    ObjectHolder.Execute(Handle, [this] (Object** ppObject)
    {
        if (Object* pObject = *ppObject)
        {
            if (pObject->Lock)
                return;

            if (NSBContext* pThread = dynamic_cast<NSBContext*>(pObject))
                RemoveThread(pThread);
            else
                RemoveThreads(pObject);

            delete pObject;
            *ppObject = nullptr;
        }
    });
}

void NSBInterpreter::ClearParams()
{
    Params.Reset();
}

void NSBInterpreter::SetLoop()
{
    if (Playable* pPlayable = PopPlayable())
        pPlayable->SetLoop(PopBool());
}

void NSBInterpreter::SetVolume()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Volume = PopInt();
    int32_t Tempo = PopTempo();

    if (pContext->FastForward)
        Time = 0;
    ObjectHolder.Execute(Handle, [Time, Volume, Tempo] (Object** ppObject)
    {
        if (Playable* pPlayable = dynamic_cast<Playable*>(*ppObject))
            pPlayable->SetVolume(Time, Volume, Tempo);
    });
}

void NSBInterpreter::SetLoopPoint()
{
    string Handle = PopString();
    int32_t Begin = PopInt();
    int32_t End = PopInt();

    if (Playable* pPlayable = Get<Playable>(Handle))
        pPlayable->SetLoopPoint(Begin, End);
}

void NSBInterpreter::CreateSound()
{
    string Handle = PopString();
    string Type = PopString();
    string File = PopString();

    if (File.substr(File.size() - 4) != ".ogg")
        File += ".ogg";

    Resource Res = sResourceMgr->GetResource(File);
    if (!Res.IsValid())
        return;

    int32_t Volume = 0;
    if (Type == "BGM")
        Volume = GetInt("#SYSTEM_sound_volume_bgm");
    else if (Type == "SE")
        Volume = GetInt("#SYSTEM_sound_volume_se");
    else if (Type == "VOICE")
        Volume = GetInt("#SYSTEM_sound_volume_voice");
    else
        return;

    Playable* pPlayable = new Playable(Res);
    pPlayable->IsBGM = Type == "BGM";
    pPlayable->SetVolume(0, Volume, -1);
    if (getenv("NP_SND_DBG"))
        fprintf(stderr, "[snd] create handle=%s type=%s file=%s playable=%p\n",
                Handle.c_str(), Type.c_str(), File.c_str(), (void*)pPlayable);
    ObjectHolder.Write(Handle, pPlayable);
}

void NSBInterpreter::RemainTime()
{
    Playable* pPlayable = PopPlayable();
    PushInt(pPlayable ? pPlayable->RemainTime() : 0);
}

void NSBInterpreter::CreateMovie()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    /*NSBPosition X = */PopPos();
    /*NSBPosition Y = */PopPos();
    bool Loop = PopBool();
    bool Alpha = PopBool();
    string File = PopString();

    Movie* pMovie = new Movie(File, pWindow, Priority, Alpha, true);
    pMovie->SetLoop(Loop);
    pWindow->AddTexture(pMovie);
    ObjectHolder.Write(Handle, pMovie);
}

void NSBInterpreter::DurationTime()
{
    Playable* pPlayable = PopPlayable();
    PushInt(pPlayable ? pPlayable->DurationTime() : 0);
}

void NSBInterpreter::SetFrequency()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Frequency = PopInt();
    int32_t Tempo = PopTempo();

    if (Playable* pPlayable = Get<Playable>(Handle))
        pPlayable->SetFrequency(Time, Frequency, Tempo);
}

void NSBInterpreter::SetPan()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t Pan = PopInt();
    int32_t Tempo = PopTempo();

    if (Playable* pPlayable = Get<Playable>(Handle))
        pPlayable->SetPan(Time, Pan, Tempo);
}

void NSBInterpreter::SetAlias()
{
    string Handle = PopString();
    string Alias = PopString();
    ObjectHolder.WriteAlias(Handle, Alias);
}

void NSBInterpreter::CreateName()
{
    ObjectHolder.Write(PopString(), new Name);
}

void NSBInterpreter::CreateWindow()
{
    string Handle = PopString();

    Window_t* pWindow = new Window_t;
    pWindow->Priority = PopInt();
    pWindow->X = PopInt();
    pWindow->Y = PopInt();
    pWindow->Width = PopInt();
    pWindow->Height = PopInt();
    /*bool unk = */PopBool();

    ObjectHolder.Write(Handle, pWindow);
}

void NSBInterpreter::CreateChoice()
{
    string Handle = PopString();
    ObjectHolder.Execute(Handle, [this](Object** ppObject)
    {
        if (Object* pObject = *ppObject)
        {
            if (NSBContext* pThread = dynamic_cast<NSBContext*>(pObject))
                RemoveThread(pThread);
            else
                RemoveThreads(pObject);
            delete pObject;
            *ppObject = nullptr;
        }
    });
    ObjectHolder.Write(Handle, new Choice);

    for (int i = 1; i < pContext->GetNumParams(); ++i)
        PopInt();
}

/*
 * Check if selected (see: Select) event satisfies a case.
 * In practise, this checks if button was clicked, and if so, jumps to
 * beginning of case code block. Otherwise, jump over the case block.
 * After the code block is executed, no other cases will be checked.
 *
 * NOTE: For unknown reason, MAGIC_CASE's last parameter is label which
 * points to beginning of the case, even though case code block always
 * begins on the next line of code.
 * */
void NSBInterpreter::Case()
{
    bool Choose = false;
    if (Choice* pChoice = Get<Choice>(pContext->GetParam(0)))
    {
        Choose = pChoice->IsSelected(Event);
        pChoice->Reset();
        if (getenv("NP_CHOICE_DBG"))
            fprintf(stderr, "[case] %s ev=%x choose=%d\n", pContext->GetParam(0).c_str(), Event.type, Choose);
    }
    else if (getenv("NP_CHOICE_DBG"))
    {
        fprintf(stderr, "[case] no choice script=%s np=%d p0='%s' p1='%s' p2='%s'\n",
                pContext->GetScriptName().c_str(), pContext->GetNumParams(),
                pContext->GetParam(0).c_str(),
                pContext->GetNumParams() > 1 ? pContext->GetParam(1).c_str() : "",
                pContext->GetNumParams() > 2 ? pContext->GetParam(2).c_str() : "");
    }

    pContext->Jump(Choose ? pContext->GetParam(2) : pContext->GetParam(1));
}

void NSBInterpreter::CaseEnd()
{
}

void NSBInterpreter::SetNextFocus()
{
    Choice* pFirst = Get<Choice>(PopString());
    Choice* pSecond = Get<Choice>(PopString());
    string Key = PopString();

    if (pFirst && pSecond)
        pFirst->SetNextFocus(pSecond, Key);
}

void NSBInterpreter::PassageTime()
{
    Playable* pPlayable = PopPlayable();
    PushInt(pPlayable ? pPlayable->PassageTime() : 0);
}

void NSBInterpreter::ParseText()
{
    string Handle = pContext->GetParam(0);
    string Box = pContext->GetParam(1);
    string XML = pContext->GetParam(2);

    ObjectHolder.Delete(GetVar("$SYSTEM_present_text")->ToString());

    Text* pText = new Text;
    pText->CreateFromXML(XML);
    Handle = Box + "/" + Handle;
    pText->DebugName = Handle;
    int32_t TextIndex = GetInt("$SYSTEM_present_text_index") + 1;
    SetInt("$SYSTEM_present_text_index", TextIndex);
    if (pContext->FastForward && Handle == ReplayTarget &&
        (!ReplayTargetTextIndex || TextIndex == ReplayTargetTextIndex))
    {
        if (getenv("NP_TEX_DBG"))
            fprintf(stderr, "[load] replay target handle=%s index=%d\n",
                    Handle.c_str(), TextIndex);
        pContext->FastForward = false;
        ReplayTargetRestorePending = true;
    }
    SetString("$SYSTEM_present_text", Handle);
    ObjectHolder.Write(Handle, pText);
}

void NSBInterpreter::LoadText()
{
    /*string unk = */PopString();
    /*string BoxHandle = */PopString();
    string TextHandle = PopString();
    int32_t Width = PopInt();
    /*int32_t Height = */PopInt();
    /*int32_t LetterSpacing = */PopInt();
    /*int32_t LineSpacing = */PopInt();

    if (Text* pText = Get<Text>(TextHandle))
    {
        pText->SetColor(Text::dInColor);
        pText->SetCharacterSize(Text::dSize);
        pText->SetPriority(20002);
        pText->SetWrap(Width);
        pText->Advance();
        pWindow->AddTexture(pText);
        UpdateDialogueTextVisibility();
    }
}

void NSBInterpreter::WaitText()
{
    string Handle = PopString();
    int32_t Time = PopInt();

    if (Text* pText = Get<Text>(Handle))
        if (!SkipHack)
            pContext->WaitText(pText, Time);
}

void NSBInterpreter::LockVideo()
{
    /*bool Lock = */PopBool();
}

void NSBInterpreter::Save()
{
    int Slot = PopInt();
    if (Slot == 0)
    {
        SaveGlobals();
        return;
    }
    if (getenv("NP_TEX_DBG"))
        fprintf(stderr, "[save] slot=%d target=%s index=%d\n", Slot,
                GetString("$SYSTEM_present_text").c_str(), GetInt("$SYSTEM_present_text_index"));

    Npa::Buffer SaveData;
    uint32_t NumVars = 0;
    for (auto& var : VariableHolder.Cache)
        if (var.first.empty() || var.first.front() != '#')
            NumVars++;
    SaveData.Write<uint32_t>(NumVars);
    vector<pair<string, Variable*> > Arrays;
    for (auto& var : VariableHolder.Cache)
    {
        if (var.first.front() == '#')
            continue;

        SaveData.WriteStr32(NpaFile::FromUtf8(var.first));
        SaveData.WriteStr32(NpaFile::FromUtf8(var.first));
        SaveData.Write<uint32_t>(var.second->GetTag());
        SaveData.Write<int32_t>(var.second->IsInt() ? var.second->ToInt() : 0);
        SaveData.Write<float>(var.second->IsFloat() ? var.second->ToFloat() : 0);
        SaveData.WriteStr32(NpaFile::FromUtf8(var.second->IsString() ? var.second->ToString() : ""));
        SaveData.Write<bool>(0); // unk - maybe bool? 4?
        SaveData.WriteStr32(NpaFile::FromUtf8("")); // TODO: arrayref?
        if (!var.second->Assoc.empty())
            Arrays.push_back(var);
    }
    SaveData.Write<uint32_t>(Arrays.size());
    for (auto& arr : Arrays)
    {
        SaveData.WriteStr32(NpaFile::FromUtf8(arr.first));
        SaveData.Write<uint32_t>(arr.second->Assoc.size());
        for (auto& i : arr.second->Assoc)
            SaveData.WriteStr32(NpaFile::ToUtf8(i.first));
    }
    SaveGlobals();
    string SavePath = GetString("#SYSTEM_save_path");
    string Filename = (boost::format("%s/%04d.npf") % SavePath % Slot).str();
    char* Encrypted = NpaFile::Encrypt(SaveData.GetData(), SaveData.GetSize());
    fs::WriteFileDirectory(Filename, Encrypted, SaveData.GetSize());

    Image Screenshot;
    Screenshot.LoadScreen(pWindow);
    if (Screenshot.GetWidth() > 0 && Screenshot.GetHeight() > 0)
    {
        const int Width = 160, Height = 90;
        vector<uint8_t> Pixels(Width * Height * 4);
        uint8_t* Source = Screenshot.GetPixels();
        for (int y = 0; y < Height; ++y)
            for (int x = 0; x < Width; ++x)
            {
                int X0 = x * Screenshot.GetWidth() / Width;
                int X1 = (x + 1) * Screenshot.GetWidth() / Width;
                int Y0 = y * Screenshot.GetHeight() / Height;
                int Y1 = (y + 1) * Screenshot.GetHeight() / Height;
                uint32_t Sum[4] = {0, 0, 0, 0};
                uint32_t Count = 0;
                for (int sy = Y0; sy < Y1; ++sy)
                    for (int sx = X0; sx < X1; ++sx)
                    {
                        const uint8_t* Pixel = Source + ((Screenshot.GetHeight() - 1 - sy) *
                                                         Screenshot.GetWidth() + sx) * 4;
                        for (int c = 0; c < 4; ++c)
                            Sum[c] += Pixel[c];
                        Count++;
                    }
                for (int c = 0; c < 4; ++c)
                    Pixels[(y * Width + x) * 4 + c] = Sum[c] / Count;
            }
        png_image ImageData = {};
        ImageData.version = PNG_IMAGE_VERSION;
        ImageData.width = Width;
        ImageData.height = Height;
        ImageData.format = PNG_FORMAT_RGBA;
        png_alloc_size_t PngSize = 0;
        png_image_write_to_memory(&ImageData, nullptr, &PngSize, 0, Pixels.data(), 0, nullptr);
        vector<uint8_t> Png(PngSize);
        if (png_image_write_to_memory(&ImageData, Png.data(), &PngSize, 0, Pixels.data(), 0, nullptr))
            fs::WriteFileDirectory((boost::format("%s/%04d/thum.npf") % SavePath % Slot).str(),
                                   reinterpret_cast<const char*>(Png.data()), PngSize);
        png_image_free(&ImageData);
    }
}

void NSBInterpreter::RestoreVariables(Npa::Buffer& SaveData, bool GlobalsOnly)
{
    uint32_t NumVars = SaveData.Read<uint32_t>();
    for (uint32_t i = 0; i < NumVars; ++i)
    {
        string Name = NpaFile::ToUtf8(SaveData.ReadStr32());
        SaveData.ReadStr32();
        uint32_t Tag = SaveData.Read<uint32_t>();
        int32_t IntValue = SaveData.Read<int32_t>();
        float FloatValue = SaveData.Read<float>();
        string StringValue = NpaFile::ToUtf8(SaveData.ReadStr32());
        SaveData.Read<bool>();
        SaveData.ReadStr32();
        bool IsGlobal = !Name.empty() && Name.front() == '#';
        if ((GlobalsOnly != IsGlobal) || Name == "#SYSTEM_save_path")
            continue;
        Variable* Value = nullptr;
        if (Tag == 1)
            Value = Variable::MakeInt(IntValue);
        else if (Tag == 2)
            Value = Variable::MakeFloat(FloatValue);
        else if (Tag == 3)
            Value = Variable::MakeString(StringValue);
        if (Value)
            SetVar(Name, Value);
    }

    uint32_t NumArrs = SaveData.Read<uint32_t>();
    for (uint32_t i = 0; i < NumArrs; ++i)
    {
        string Name = NpaFile::ToUtf8(SaveData.ReadStr32());
        uint32_t NumElems = SaveData.Read<uint32_t>();
        Variable* Array = GetVar(Name);
        if ((Name.empty() || Name.front() != '#') == GlobalsOnly || Name == "#SYSTEM_save_path")
        {
            for (uint32_t j = 0; j < NumElems; ++j)
                SaveData.ReadStr32();
            continue;
        }
        Array->Assoc.clear();
        for (uint32_t j = 0; j < NumElems; ++j)
            Array->Assoc[NpaFile::ToUtf8(SaveData.ReadStr32())] = j;
    }
}

void NSBInterpreter::SaveGlobals()
{
    Npa::Buffer SaveData;
    uint32_t NumVars = 0;
    for (auto& var : VariableHolder.Cache)
        if (!var.first.empty() && var.first.front() == '#' && var.first != "#SYSTEM_save_path")
            NumVars++;
    SaveData.Write<uint32_t>(NumVars);
    vector<pair<string, Variable*> > Arrays;
    for (auto& var : VariableHolder.Cache)
    {
        if (var.first.empty() || var.first.front() != '#' || var.first == "#SYSTEM_save_path")
            continue;
        SaveData.WriteStr32(NpaFile::FromUtf8(var.first));
        SaveData.WriteStr32(NpaFile::FromUtf8(var.first));
        SaveData.Write<uint32_t>(var.second->GetTag());
        SaveData.Write<int32_t>(var.second->IsInt() ? var.second->ToInt() : 0);
        SaveData.Write<float>(var.second->IsFloat() ? var.second->ToFloat() : 0);
        SaveData.WriteStr32(NpaFile::FromUtf8(var.second->IsString() ? var.second->ToString() : ""));
        SaveData.Write<bool>(0);
        SaveData.WriteStr32(NpaFile::FromUtf8(""));
        if (!var.second->Assoc.empty())
            Arrays.push_back(var);
    }
    SaveData.Write<uint32_t>(Arrays.size());
    for (auto& arr : Arrays)
    {
        SaveData.WriteStr32(NpaFile::FromUtf8(arr.first));
        SaveData.Write<uint32_t>(arr.second->Assoc.size());
        for (auto& i : arr.second->Assoc)
            SaveData.WriteStr32(NpaFile::FromUtf8(i.first));
    }
    string Filename = GetString("#SYSTEM_save_path") + "/global.npf";
    char* Encrypted = NpaFile::Encrypt(SaveData.GetData(), SaveData.GetSize());
    fs::WriteFileDirectory(Filename, Encrypted, SaveData.GetSize());
}

void NSBInterpreter::LoadGlobals()
{
    string Filename = GetString("#SYSTEM_save_path") + "/global.npf";
    if (!fs::Exists(Filename))
        return;
    uint32_t Size = 0;
    char* Data = fs::ReadFile(Filename, Size);
    if (!Data)
        return;
    Npa::Buffer SaveData(NpaFile::Decrypt(Data, Size), Size);
    RestoreVariables(SaveData, true);
}

void NSBInterpreter::DeleteSaveFile()
{
    string Filename = PopSave();
    fs::DeleteFile(Filename);
    fs::DeleteDirectory(Filename.substr(0, Filename.size() - 4));
}

void NSBInterpreter::Conquest()
{
    /*string unk = */PopString();
    /*string unk = */PopString();
    /*string unk = */PopBool();

    // [HACK]
    PushInt(0);
}

void NSBInterpreter::ClearScore()
{
    /*string unk = */PopString();
}

void NSBInterpreter::ClearBacklog()
{
}

void NSBInterpreter::SetFont()
{
    Text::dFont = PopString();
    Text::dSize = PopInt();
    Text::dInColor = PopColor();
    Text::dOutColor = PopColor();
    Text::dWeight = PopInt();
    Text::dAlign = PopString();
}

void NSBInterpreter::SetShortcut()
{
    string Key = PopString();
    string Script = PopString();
    Shortcuts.push_back({SDLK_a + Key[0] - 'A', Script});
}

void NSBInterpreter::CreateClipTexture()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    NSBPosition X1 = PopPos();
    NSBPosition Y1 = PopPos();
    int32_t X2 = PopInt();
    int32_t Y2 = PopInt();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    string Source = PopString();

    Texture* pTexture = new Texture;
    if (Source.size() < 4 || Source[Source.size() - 4] != '.')
        pTexture->CreateFromImageClip(Get<Image>(Source), X2, Y2, Width, Height);
    else
        pTexture->CreateFromFileClip(Source, X2, Y2, Width, Height);

    pTexture->Move(X1(pTexture->GetWidth()), Y1(pTexture->GetHeight()));
    pTexture->SetPriority(Priority);

    pWindow->AddTexture(pTexture);
    ObjectHolder.Write(Handle, pTexture);
}

void NSBInterpreter::ExistSave()
{
    PushInt(fs::Exists(PopSave()));
}

void NSBInterpreter::WaitAction()
{
    string Handle = PopString();
    int32_t Time = pContext->GetNumParams() == 2 ? PopInt() : -1;

    if (Object* pObject = GetObject(Handle))
        pContext->WaitAction(pObject, Time);
}

void NSBInterpreter::Load()
{
    int Slot = PopInt();
    if (pContext->GetNumParams() > 1)
        PopBool();
    if (Slot == 0)
    {
        LoadGlobals();
        return;
    }
    string Filename = (boost::format("%s/%04d.npf") % GetString("#SYSTEM_save_path") % Slot).str();
    uint32_t Size;
    char* pData = fs::ReadFile(Filename, Size);
    if (!pData)
        return;
    Npa::Buffer SaveData(NpaFile::Decrypt(pData, Size), Size);
    LastTextWindowSymbol.clear();
    DialogueMenuWasOpen = false;
    MenuTextureSnapshotCaptured = false;
    MenuTextureOpacities.clear();
    RestoreVariables(SaveData, false);
    string Target = GetString("$SYSTEM_present_text");
    ReplayTargetTextIndex = GetInt("$SYSTEM_present_text_index");
    ReplayGameName = GetString("$GameName");
    if (getenv("NP_TEX_DBG"))
        fprintf(stderr, "[load] target handle=%s index=%d\n",
                Target.c_str(), ReplayTargetTextIndex);
    SetInt("$SYSTEM_present_text_index", 0);
    SetString("$SYSTEM_menu_enable", "false");
    SetString("$SYSTEM_menu_load_enable", "false");
    SetString("$SYSTEM_menu_save_enable", "false");
    SetString("$SYSTEM_menu_config_enable", "false");
    SetString("$SYSTEM_backlog_enable", "false");
    SetString("$SYSTEM_backselect_enable", "false");
    SetString("$SYSTEM_menu_close_enable", "false");
    SetString("$SYSTEM_reset_enable", "false");
    SetString("$LOCAL_hmode_enable", "false");
    SetString("$BOX_Init", "false");
    SetString("$BGM_Init", "false");
    SetInt("$GameContiune", 1);
    SetInt("$GameStart", 1);
    pContext->Reset();
    for (auto i = Threads.begin(); i != Threads.end(); )
    {
        NSBContext* pThread = *i;
        if (pThread != pContext)
        {
            pThread->Reset();
            i = Threads.erase(i);
        }
        else
            ++i;
    }
    ThreadsModified = true;
    Playable::StopAll();
    ObjectHolder.DetachExcept(pContext);
    pContext->Reset();
    pContext->FastForward = true;
    ReplayTarget = Target;
    ReplayTargetRestorePending = false;
    HasLastTextOrientation = false;
    TextOrientationTransitionPending = false;
    if (pBootScript)
        pContext->Call(pBootScript, "chapter.main");
}

void NSBInterpreter::SetBacklog()
{
    /*string Text = */PopString();
    /*string Voice = */PopString();

    // [WORKAROUND] In JAST the third parameter may be an integer
    Variable::Destroy(PopVar());
    ///*string Name = */PopString();
}

void NSBInterpreter::CreateText()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();
    NSBPosition Width = PopPos();
    /*NSBPosition Height = */PopPos();
    string String = PopString();

    Text* pText = new Text;
    pText->SetWrap(Width(0));
    pText->CreateFromString(String);
    pText->SetPriority(Priority);
    pText->SetPosition(X(pText->GetWidth()), Y(pText->GetHeight()));
    pText->DebugName = Handle;

    pWindow->AddTexture(pText);
    ObjectHolder.Write(Handle, pText);
}

void NSBInterpreter::AtExpression()
{
    Variable* pVar = PopVar();
    pVar->Relative = true;
    PushVar(pVar);
}

void NSBInterpreter::Random()
{
    PushInt(random() % PopInt());
}

void NSBInterpreter::CreateEffect()
{
    string Handle = PopString();
    /*int32_t Priority = */PopInt();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();
    /*int32_t Width = */PopInt();
    /*int32_t Height = */PopInt();
    /*int32_t Effect = */PopEffect();
}

void NSBInterpreter::SetTone()
{
    if (Texture* pTexture = PopTexture())
        pTexture->SetTone(PopTone());
}

void NSBInterpreter::DateTime()
{
    time_t t = time(nullptr);
    tm* tms = localtime(&t);
    PopVar()->Set(tms->tm_year + 1900);
    PopVar()->Set(tms->tm_mon + 1);
    PopVar()->Set(tms->tm_mday);
    PopVar()->Set(tms->tm_hour);
    PopVar()->Set(tms->tm_min);
    PopVar()->Set(tms->tm_sec);
}

void NSBInterpreter::Shake()
{
    string Handle = PopString();
    int32_t Time = PopInt();
    int32_t XWidth = PopInt();
    int32_t YWidth = PopInt();
    /*int32_t unk1 = */PopInt();
    /*int32_t unk2 = */PopInt();
    /*int32_t unk3 = */PopInt();
    /*int32_t Tempo = */PopTempo();
    bool Wait = PopBool();

    ObjectHolder.Execute(Handle, [Time, XWidth, YWidth] (Object** ppObject)
    {
        if (Texture* pTexture = dynamic_cast<Texture*>(*ppObject))
            pTexture->Shake(XWidth, YWidth, Time);
    });

    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::MoviePlay()
{
    string File = PopString();
    /*bool unk = */PopBool();
}

void NSBInterpreter::SetStream()
{
    string Handle = PopString();
    /*int32_t unk = */PopInt();
}

void NSBInterpreter::WaitPlay()
{
    Playable* pPlayable = PopPlayable();
    /*string unk = */PopString();

    pContext->Wait(pPlayable ? pPlayable->RemainTime() : 0);
}

void NSBInterpreter::WaitFade()
{
    Texture* pTexture = PopTexture();
    /*string unk = */PopString();

    pContext->Wait(pTexture ? pTexture->RemainFade() : 0);
}

void NSBInterpreter::SoundAmplitude()
{
    string Handle = PopString();
    /*string unk = */PopString();

    // [HACK]
    PushInt(0);
}

void NSBInterpreter::Rotate()
{
    Texture* pTexture = PopTexture();
    int32_t Time = PopInt();
    /*NSBPosition X = */PopRelative();
    /*NSBPosition Y = */PopRelative();
    NSBPosition Z = PopRelative();
    int32_t Tempo = PopTempo();
    bool Wait = PopBool();

    if (pTexture)
    {
        int32_t Target = Z(0, pTexture->GetAngle());
        if (Text* pText = dynamic_cast<Text*>(pTexture))
        {
            bool Tategaki = Text::dAlign.find("DOWN") != string::npos ||
                            Text::dAlign.find("down") != string::npos;
            if (Tategaki)
                pText->SetVertical(((Target % 180) + 180) % 180 == 90);
        }
        if (pContext->FastForward)
            Time = 0;
        pTexture->Rotate(Target, Time, Tempo);
    }
    if (Wait)
        pContext->Wait(Time);
}

void NSBInterpreter::Message()
{
    static const map<string, int> buttonret =
    {
        {"CANCEL", 0},
        {"OK", 1},
        {"YES", 2},
        {"NO", 3},
        {"ABORT", 4},
        {"RETRY", 5},
        {"IGNORE", 6}
    };

    static const string buttonstr[] =
    {
        "Cancel", "OK", "Yes", "No", "Abort", "Retry", "Ignore"
    };

    static const map<string, vector<string> > type
    {
        {"OKCANCEL", {"OK", "CANCEL"}},
        {"YESNO", {"YES", "NO"}},
        {"YESNOCANCEL", {"YES", "NO", "CANCEL"}},
        {"RETRYCANCEL", {"RETRY", "CANCEL"}},
        {"ABORTRETRYIGNORE", {"ABORT", "RETRY", "IGNORE"}}
    };

    static const map<string, Uint32> icon =
    {
        {"STOP", SDL_MESSAGEBOX_ERROR},
        {"INFORMATION", SDL_MESSAGEBOX_INFORMATION},
        {"EXCLAMATION", SDL_MESSAGEBOX_WARNING},
        {"QUESTION", SDL_MESSAGEBOX_INFORMATION}
    };

    string Title = PopString();
    string Text = PopString();
    string Type = PopString();
    string Icon = PopString();

    vector<SDL_MessageBoxButtonData> Buttons;
    for (const string& btnstr : type.at(Type))
    {
        int ButtonRet = buttonret.at(btnstr);
        Buttons.push_back({0, ButtonRet, buttonstr[ButtonRet].c_str()});
    }

    SDL_MessageBoxData Data =
    {
        icon.at(Icon),
        NULL,
        Title.c_str(),
        Text.c_str(),
        (int)Buttons.size(),
        &Buttons[0],
        NULL
    };
    int RetVal;
    SDL_ShowMessageBox(&Data, &RetVal);
    PushInt(RetVal);
}

void NSBInterpreter::Integer()
{
    PushVar(PopVar());
}

void NSBInterpreter::CreateScrollbar()
{
    string Handle = PopString();
    int32_t Priority = PopInt();
    int32_t X1 = PopInt();
    int32_t Y1 = PopInt();
    int32_t X2 = PopInt();
    int32_t Y2 = PopInt();
    int32_t Value = PopInt();
    string Type = PopString();
    string Filename = PopString();

    Texture* pTexture = new Texture;
    Scrollbar* pScrollbar = new Scrollbar(pTexture, X1, Y1, X2, Y2, 0, 1, Type, "");
    pScrollbar->SetValue(Value);
    pTexture->CreateFromFile(Filename);
    pTexture->Move(X1, Y1);
    pTexture->SetPriority(Priority);
    pWindow->AddTexture(pTexture);
    ObjectHolder.Write(Handle, pScrollbar);
}

void NSBInterpreter::SetScrollbarValue()
{
    string Handle = PopString();
    int32_t Value = PopInt();
    if (Scrollbar* pScrollbar = Get<Scrollbar>(Handle))
        pScrollbar->SetValue(Value);
    else
        LogMissingScrollbar(Handle);
}

void NSBInterpreter::SetScrollbarWheelArea()
{
    Scrollbar* pScrollbar = PopScrollbar();
    int32_t X = PopInt();
    int32_t Y = PopInt();
    int32_t Width = PopInt();
    int32_t Height = PopInt();
    pScrollbar->SetWheelArea(X, Y, Width, Height);
}

void NSBInterpreter::ScrollbarValue()
{
    string Handle = PopString();
    if (Scrollbar* pScrollbar = Get<Scrollbar>(Handle))
        PushInt(pScrollbar->GetValue());
    else
    {
        LogMissingScrollbar(Handle);
        PushInt(0);
    }
}

void NSBInterpreter::CreateStencil()
{
    string Handle = PopString();
    /*int32_t unk = */PopInt();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();
    /*int32_t unk = */PopInt();
    string Filename = PopString();
    /*bool unk = */PopBool(); // Maybe Inheritance?

    // Hack
    ObjectHolder.Write(Handle, new Name);
}

void NSBInterpreter::CreateMask()
{
    string Handle = PopString();
    /*int32_t Priority = */PopInt();
    NSBPosition X = PopPos();
    NSBPosition Y = PopPos();
    string Filename = PopString();
    /*bool Inheritance = */PopBool();

    // Hack
    ObjectHolder.Write(Handle, new Name);
}
