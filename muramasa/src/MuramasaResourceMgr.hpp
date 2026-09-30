#ifndef MURAMASA_RESOURCE_MGR_HPP
#define MURAMASA_RESOURCE_MGR_HPP

#include "ResourceMgr.hpp"
#include <string>
#include <vector>

class INipaFile;

class MuramasaResourceMgr : public ResourceMgr
{
public:
    MuramasaResourceMgr();
    ~MuramasaResourceMgr();
    ScriptFile* ReadScriptFile(const std::string& Path);
    char* Read(std::string Path, uint32_t& Size);
    Resource GetResource(std::string Path);
};

#endif
