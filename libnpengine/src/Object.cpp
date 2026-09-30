#include "Object.hpp"
void ObjectHolder_t::DetachExcept(Object* Keep)
{
    for (auto i = Cache.begin(); i != Cache.end(); )
    {
        Object* pObject = i->second;
        if (pObject != Keep)
            delete pObject;
        i = Cache.erase(i);
    }
    Aliases.clear();
}
#include "NSBContext.hpp"

Object::~Object()
{
    std::vector<NSBContext*> Waiting = Waiters;
    Waiters.clear();
    for (NSBContext* pContext : Waiting)
        pContext->OnWaitedObjectDeleted(this);
}
