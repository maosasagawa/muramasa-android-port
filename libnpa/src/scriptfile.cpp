#include <cassert>
#include "scriptfile.hpp"
#include "npafile.hpp"
#include "fscommon.hpp"
#include "buffer.hpp"
#include "nsbmagic.hpp"
#include <cstring>
#include <algorithm>
using namespace Npa;

ScriptFile::ScriptFile(const string& Name, const char* NssData, uint32_t NssSize, bool Utf8Source_) :
Name(Name), Utf8Source(Utf8Source_)
{
    ReadNss(NssData, NssSize);
}

ScriptFile::ScriptFile(const string& Name, char* NsbData, uint32_t NsbSize, char* MapData, uint32_t MapSize) :
Name(Name)
{
    Buffer NsbBuffer(NsbData, NsbSize);
    Buffer MapBuffer(MapData, MapSize);
    ReadNsb(NsbBuffer, MapBuffer);
}

ScriptFile::ScriptFile(const string& Name, FileType Type) :
Name(Name)
{
    switch (Type)
    {
        case NSS: OpenNss(Name); break;
        case NSB: OpenNsb(Name); break;
        default: assert(false);
    }
}

void ScriptFile::OpenNss(const string& Name)
{
    uint32_t NssSize;
    char* NssData = fs::ReadFile(Name, NssSize);
    ReadNss(NssData, NssSize);
    delete[] NssData;
}

void ScriptFile::OpenNsb(string Name)
{
    string MapName = string(Name, 0, Name.size() - 3) + "map";
    uint32_t NsbSize, MapSize;
    char* NsbData = fs::ReadFile(Name, NsbSize);
    char* MapData = fs::ReadFile(MapName, MapSize);
    Buffer NsbBuffer(NsbData, NsbSize);
    Buffer MapBuffer(MapData, MapSize);
    ReadNsb(NsbBuffer, MapBuffer);
}

void ScriptFile::ReadNss(const char* NssData, uint32_t NssSize)
{
    Buffer NsbBuffer, MapBuffer;
    Nss::Compile(NssData, NssSize, &NsbBuffer, &MapBuffer);
    ReadNsb(NsbBuffer, MapBuffer);
}

void ScriptFile::ReadNsb(Npa::Buffer& NsbData, Npa::Buffer& MapData)
{
    uint32_t Entry;
    uint16_t NumParams;

    // Read source code lines
    while (!NsbData.EndOfBuffer())
    {
        Entry = NsbData.Read<uint32_t>() - 1; // Start counting at zero
        Source.resize(Source.size() + 1);
        Source[Entry].Magic = NsbData.Read<uint16_t>();
        NumParams = NsbData.Read<uint16_t>();
        Source[Entry].Params.reserve(NumParams);

        // Read parameters
        for (uint16_t i = 0; i < NumParams; ++i)
        {
            string Param = NsbData.ReadStr32();
            Source[Entry].Params.push_back(Utf8Source ? Param : NpaFile::ToUtf8(Param));
        }
    }

    // Read symbols
    while (!MapData.EndOfBuffer())
    {
        NsbData.Read(&Entry, sizeof(uint32_t), MapData.Read<uint32_t>());
        string RawLabel = MapData.ReadStr16();
        string Label = Utf8Source ? RawLabel : NpaFile::ToUtf8(RawLabel);
        if (Label.substr(0, 7) == "include")
            Includes.push_back(Label.substr(8));
        else
        {
            transform(Label.begin(), Label.end(), Label.begin(), ::tolower);
            Symbols[Label] = Entry - 1;
        }
    }
}

uint32_t ScriptFile::GetSymbol(const string& Symbol)
{
    string Lower = Symbol;
    transform(Lower.begin(), Lower.end(), Lower.begin(), ::tolower);
    auto iter = Symbols.find(Lower);
    if (iter != Symbols.end())
        return iter->second;
    return NSB_INVALIDE_LINE;
}

string Line::Stringify() const
{
    string String = Nsb::StringifyMagic(Magic) + string("(");
    for (unsigned i = 0; i < Params.size(); ++i)
    {
        String += Params[i];
        String += ((i == Params.size() - 1) ? "" : ", ");
    }
    return String + ");";
}
