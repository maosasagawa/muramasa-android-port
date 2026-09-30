#include <unistd.h>
#ifdef __ANDROID__
#include <SDL_main.h>
#include <SDL_system.h>
#include <SDL_rwops.h>
#include <android/log.h>
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, "muramasa", __VA_ARGS__)
#else
#define ALOGI(...) do {} while (0)
#endif
#include "MuramasaWindow.hpp"
#include "MuramasaInterpreter.hpp"
#include "MuramasaResourceMgr.hpp"
#include "npafile.hpp"
#include <iostream>
#include <cstring>
using namespace std;

int main(int argc, char** argv)
{
    const char* script = "nss/0_boot.nss";
    const char* locale = "zh_CN.GBK";
    const char* dir = nullptr;
    bool debug = false;

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--script") && i + 1 < argc)
            script = argv[++i];
        else if (!strcmp(argv[i], "--locale") && i + 1 < argc)
            locale = argv[++i];
        else if (!strcmp(argv[i], "--dir") && i + 1 < argc)
            dir = argv[++i];
        else if (!strcmp(argv[i], "--debug"))
            debug = true;
    }

#ifdef __ANDROID__
    if (!dir)
    {
        static string IntDir = string(SDL_AndroidGetInternalStoragePath()) + "/game";
        static string ExtDir = string(SDL_AndroidGetExternalStoragePath()) + "/game";
        dir = IntDir.c_str();
        if (access((IntDir + "/cg.npa").c_str(), R_OK) != 0)
            dir = ExtDir.c_str();
    }

    // Ship the CJK font as an APK asset; extract once so FontRenderer can fopen it.
    static string FontPath = string(SDL_AndroidGetInternalStoragePath()) + "/font.ttc";
    if (access(FontPath.c_str(), R_OK) != 0)
        if (SDL_RWops* RW = SDL_RWFromFile("font.ttc", "rb"))
        {
            Sint64 Size = SDL_RWsize(RW);
            if (FILE* f = fopen(FontPath.c_str(), "wb"))
            {
                char Buf[65536];
                while (Size > 0)
                {
                    size_t N = SDL_RWread(RW, Buf, 1, Size > (Sint64)sizeof(Buf) ? sizeof(Buf) : (size_t)Size);
                    if (!N) break;
                    fwrite(Buf, 1, N, f);
                    Size -= N;
                }
                fclose(f);
            }
            SDL_RWclose(RW);
        }
    setenv("NP_FONT", FontPath.c_str(), 1);
#endif
    if (dir && chdir(dir) != 0)
    {
        ALOGI("cannot chdir to %s", dir);
        cerr << "cannot chdir to " << dir << endl;
        return 1;
    }
    ALOGI("muramasa starting, dir=%s script=%s", dir ? dir : "(cwd)", script);

    // Muramasa .nss scripts are GBK (Chinese patch build); Japanese original is CP932.
    NpaFile::SetLocale(locale);

    sResourceMgr = new MuramasaResourceMgr;
    MuramasaWindow* pWindow = new MuramasaWindow;
    MuramasaInterpreter* pInterpreter = new MuramasaInterpreter(pWindow);

    try
    {
        pInterpreter->ExecuteScript(script);
        if (debug)
            pInterpreter->StartDebugger();
    }
    catch (const std::exception& e)
    {
        ALOGI("exception: %s", e.what());
        throw;
    }
    catch (...)
    {
        ALOGI("unknown exception");
        throw;
    }
    ALOGI("script %s finished", script);

    pWindow->SetInterpreter(pInterpreter);
    pWindow->Run();
    delete pWindow;
    delete sResourceMgr;
    return 0;
}
