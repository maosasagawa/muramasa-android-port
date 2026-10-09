#include <unistd.h>
#ifdef __ANDROID__
#include <SDL_main.h>
#include <SDL_messagebox.h>
#include <SDL_system.h>
#include <SDL_rwops.h>
#include <android/log.h>
#include <sys/stat.h>
#include <errno.h>
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, "muramasa", __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, "muramasa", __VA_ARGS__)
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

#ifdef __ANDROID__
static bool MkdirsFor(const string& FilePath)
{
    string P = FilePath;
    for (size_t pos = P.find('/'); pos != string::npos; pos = P.find('/', pos + 1))
    {
        P[pos] = '\0';
        if (pos > 0 && access(P.c_str(), F_OK) != 0 &&
            mkdir(P.c_str(), 0755) != 0 && errno != EEXIST)
            return false;
        P[pos] = '/';
    }
    return true;
}

// When the APK bundles game data under assets/gamedata/, manifest.txt lists each
// file as "size<TAB>relpath"; copy missing/mismatched files to internal storage.
static bool ExtractBundledGameData(const string& TargetRoot)
{
    SDL_RWops* Manifest = SDL_RWFromFile("gamedata/manifest.txt", "rb");
    if (!Manifest)
        return false;
    Sint64 ManifestSize = SDL_RWsize(Manifest);
    string Text((size_t)ManifestSize, '\0');
    bool Ok = SDL_RWread(Manifest, &Text[0], 1, ManifestSize) == ManifestSize;
    SDL_RWclose(Manifest);
    if (!Ok)
        return false;

    ALOGI("Extracting bundled game data to %s", TargetRoot.c_str());
    char Buf[262144];
    size_t Line = 0;
    while (Ok && Line < Text.size())
    {
        size_t End = Text.find('\n', Line);
        if (End == string::npos)
            End = Text.size();
        string Entry = Text.substr(Line, End - Line);
        Line = End + 1;
        size_t Tab = Entry.find('\t');
        if (Tab == string::npos)
            continue;
        uint64_t Size = strtoull(Entry.c_str(), nullptr, 10);
        string Rel = Entry.substr(Tab + 1);
        if (!Rel.empty() && Rel.back() == '\r')
            Rel.pop_back();
        string Target = TargetRoot + "/" + Rel;
        struct stat St;
        if (stat(Target.c_str(), &St) == 0 && (uint64_t)St.st_size == Size)
            continue;
        SDL_RWops* Src = SDL_RWFromFile(("gamedata/" + Rel).c_str(), "rb");
        FILE* Dst = Src && MkdirsFor(Target) ? fopen(Target.c_str(), "wb") : nullptr;
        if (!Dst)
        {
            if (Src)
                SDL_RWclose(Src);
            Ok = false;
            break;
        }
        uint64_t Done = 0;
        Sint64 N;
        while ((N = SDL_RWread(Src, Buf, 1, sizeof(Buf))) > 0)
            if (fwrite(Buf, 1, (size_t)N, Dst) == (size_t)N)
                Done += (uint64_t)N;
            else
                break;
        fclose(Dst);
        SDL_RWclose(Src);
        if (Done != Size)
        {
            ALOGW("extract mismatch: %s", Rel.c_str());
            Ok = false;
        }
    }
    ALOGI("Bundled game data extraction %s", Ok ? "complete" : "FAILED");
    return Ok;
}
#endif

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
        static const string IntDir = string(SDL_AndroidGetInternalStoragePath()) + "/game";
        static const string ExtDir = string(SDL_AndroidGetExternalStoragePath()) + "/game";
        static const string SharedDir = "/storage/emulated/0/Muramasa";
        if (access((IntDir + "/cg.npa").c_str(), R_OK) != 0)
            ExtractBundledGameData(IntDir);
        const string* candidates[] = {&IntDir, &ExtDir, &SharedDir};
        const char* message =
            "把游戏文件夹(含 cg.npa)放到 内部存储/Muramasa/ 并授予“所有文件访问权限”，"
            "或放到 Android/data/org.devin.muramasa/files/game/，然后重新打开";
        auto findGameDir = [&]() -> const char* {
            for (const string* candidate : candidates)
                if (access((*candidate + "/cg.npa").c_str(), R_OK) == 0)
                    return candidate->c_str();
            return nullptr;
        };
        dir = findGameDir();
        if (!dir)
        {
            ALOGI("No readable game directory found; waiting up to 180 seconds");
            for (int second = 0; second < 180 && !dir; ++second)
            {
                sleep(1);
                dir = findGameDir();
            }
        }
        if (!dir)
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "装甲悪鬼村正", message, nullptr);
            return 1;
        }
        ALOGI("Using game directory: %s", dir);
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
#ifdef __ANDROID__
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "装甲悪鬼村正",
            "把游戏文件夹(含 cg.npa)放到 内部存储/Muramasa/ 并授予“所有文件访问权限”，"
            "或放到 Android/data/org.devin.muramasa/files/game/，然后重新打开",
            nullptr);
#endif
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
