#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "MuramasaResourceMgr.hpp"
#include "scriptfile.hpp"
#include "inipafile.hpp"
#include "npafile.hpp"
#include <vector>
#include <cstdlib>
#include <fstream>

// GameID 4 = "Muramasa" (NipaGames table in libnpa)
static const uint8_t MURAMASA_GAME_ID = 4;

static const char* ArchiveFileNames[] =
{
    "cg.npa",
    "cg2.npa",
    "nss.npa",
    "sound.npa",
    "sound2.npa",
    "system.npa",
    "voice.npa",
    "voice2.npa",
};

MuramasaResourceMgr::MuramasaResourceMgr()
{
    for (const char* name : ArchiveFileNames)
    {
        INipaFile* p = new INipaFile(name, MURAMASA_GAME_ID);
        int n = 0;
        for (auto i = p->Begin(); i != p->End(); ++i) ++n;
        fprintf(stderr, "[rmgr] %s: %d entries\n", name, n);
        if (n == 0)
        {
            delete p;
            continue;
        }
        Archives.push_back(p);
    }
}

MuramasaResourceMgr::~MuramasaResourceMgr()
{
}

ScriptFile* MuramasaResourceMgr::ReadScriptFile(const std::string& Path)
{
    // Muramasa stores plaintext .nss sources in nss.npa under flat names,
    // while scripts reference them as "nss/xxx.nss".
    std::string Flat = Path;
    size_t pos = Flat.find("nss/");
    if (pos != std::string::npos)
        Flat.erase(pos, 4);

    uint32_t NssSize;
    char* NssData = nullptr;
    std::ifstream Loose(Flat, std::ios::binary);
    if (Loose)
    {
        Loose.seekg(0, std::ios::end);
        NssSize = Loose.tellg();
        Loose.seekg(0, std::ios::beg);
        NssData = new char[NssSize];
        Loose.read(NssData, NssSize);
    }
    if (!NssData)
        NssData = Read(Flat, NssSize);
    if (!NssData)
        return nullptr;

    std::string Utf8 = NpaFile::ToUtf8(NssData, NssSize);
    delete[] NssData;
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "muramasa", "ReadScriptFile %s flat=%s nss=%u utf8=%u",
        Path.c_str(), Flat.c_str(), NssSize, (unsigned)Utf8.size());
#endif

    ScriptFile* pScript = new ScriptFile(Path, Utf8.data(), Utf8.size());
    return pScript;
}

// Muramasa scripts address resources with the archive name as the first
// path segment ("cg/sys/...", "voice/mb04/..."); our npa keys are flat,
// so fall back to the path minus its first segment.
char* MuramasaResourceMgr::Read(std::string Path, uint32_t& Size)
{
    if (char* p = ResourceMgr::Read(Path, Size))
        return p;
    size_t pos = Path.find('/');
    while (pos != std::string::npos)
    {
        Path = Path.substr(pos + 1);
        if (char* p = ResourceMgr::Read(Path, Size))
            return p;
        pos = Path.find('/');
    }
    Size = 0;
    return nullptr;
}

Resource MuramasaResourceMgr::GetResource(std::string Path)
{
    Resource R = ResourceMgr::GetResource(Path);
    if (R.IsValid())
        return R;
    size_t pos = Path.find('/');
    while (pos != std::string::npos)
    {
        Path = Path.substr(pos + 1);
        R = ResourceMgr::GetResource(Path);
        if (R.IsValid())
            return R;
        pos = Path.find('/');
    }
    return R;
}
