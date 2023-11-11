#include "cerberusregister.h"

#include <cstdlib>

#include "cerberusobject.h"
#include "cerberusutils.h"
#include "src/define.h"
#include "src/mutex/mutexlocker.h"
#include "src/thread/thread.h"

using namespace cerberus;
using namespace cerberus::core;
using namespace cerberus::message;

//=============================================================================
uint32_t CerberusRegister::findAvailableId()
{
    while (true)
    {
        uint32_t id = (uint32_t)rand();

        bool found = false;
        for (auto&& el : m_objects)
        {
            if (id == el->id())
            {
                found = true;
                break;
            }
        }

        if (id < CERBERUS_FACTORY_START_ID)
        {
            continue;
        }

        if (!found) return id;
    }
}
//=============================================================================
CerberusRegister::CerberusRegister() { srand(584239578); }
//=============================================================================
CerberusRegister& CerberusRegister::instance()
{
    static CerberusRegister reg;
    return reg;
}
//=============================================================================
void CerberusRegister::registerObj(CerberusObject* object)
{
    if (object == nullptr) return;

    auto& reg = instance();

    mutex::MutexLocker locker(reg.mutex);

    if (!object->name().empty())
    {
        for (auto& el : reg.m_objects)
        {
            if (CerberusUtils::areEqual(object->name(), el->name()))
            {
                return;
            }
        }
    }

    object->m_id = reg.findAvailableId();

    if (object->type() == CerberusObject::Thread)
    {
        reg.m_objects.push_back(object);
    }
    else if (object->type() == CerberusObject::MessageTemplate)
    {
        auto mt = new MessageTemplate(*(object->to_p<MessageTemplate>()));  // copy
        reg.m_objects.push_back(mt);
    }
    else if (object->type() == CerberusObject::Socket)
    {
        reg.m_objects.push_back(object);
    }
    else
    {
        throw cerberusIllegalArgExc("given object has no type");
    }
}
//=============================================================================
void CerberusRegister::unregisterObj(uint32_t id)
{
    if (id == CERBERUS_INVALID_ID) return;

    auto& reg = instance();

    mutex::MutexLocker locker(reg.mutex);

    for (auto it = reg.m_objects.begin(); it != reg.m_objects.end(); it++)
    {
        if ((*it)->id() == id)
        {
            debug("Unregistering %s", (*it)->toObjStr().c_str());

            if ((*it)->type() == CerberusObject::MessageTemplate)
            {
                delete *it;
            }

            reg.m_objects.erase(it);

            return;
        }
    }
}
//=============================================================================
CerberusObject* CerberusRegister::objById(uint32_t id)
{
    if (id == CERBERUS_INVALID_ID) return nullptr;

    for (auto&& el : m_objects)
    {
        if (el->id() == id)
        {
            return el;
        }
    }

    return nullptr;
}
//=============================================================================
CerberusRegister::~CerberusRegister() {}
//=============================================================================
uint32_t CerberusRegister::threadIdByName(const std::string& name)
{
    auto& reg = instance();

    mutex::MutexLocker locker(reg.mutex);

    CerberusObject* found = reg.objByName(name);

    if (found == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }
    else
    {
        if (found->type() == CerberusObject::ObjectType::Thread)
        {
            return found->id();
        }
        else
        {
            return CERBERUS_INVALID_ID;
        }
    }
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateByName(const std::string& name)
{
    auto& reg = instance();

    mutex::MutexLocker locker(reg.mutex);

    CerberusObject* found = reg.objByName(name);

    if (found == nullptr)
    {
        return message::MessageTemplate();
    }
    else
    {
        if (found->type() == CerberusObject::ObjectType::MessageTemplate)
        {
            return *(found->to_p<message::MessageTemplate>());
        }
        else
        {
            return message::MessageTemplate();
        }
    }
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateById(uint32_t id)
{
    auto& reg = instance();
    mutex::MutexLocker locker(reg.mutex);
    CerberusObject* found = reg.objById(id);

    if (found == nullptr)
    {
        return message::MessageTemplate();
    }
    else
    {
        if (found->type() == CerberusObject::ObjectType::MessageTemplate)
        {
            return message::MessageTemplate(*(found->to_p<message::MessageTemplate>()));
        }
        else
        {
            return message::MessageTemplate();
        }
    }
}
//=============================================================================
void CerberusRegister::sendMsgToObj(uint32_t id, message::cerberus_message msg)
{
    auto& reg = instance();
    mutex::MutexLocker locker(reg.mutex);
    CerberusObject* found = reg.objById(id);

    if (found->type() == CerberusObject::ObjectType::Thread)
    {
        auto thr = found->to_p<cerberus::thread::Thread>();
        thr->addMessage(msg);
    }
}
//=============================================================================
CerberusObject* CerberusRegister::objByName(const std::string& name)
{
    if (name.empty())
    {
        return nullptr;
    }

    for (auto&& el : m_objects)
    {
        if (core::CerberusUtils::areEqual(el->name(), name))
        {
            return el;
        }
    }

    return nullptr;
}
//=============================================================================
