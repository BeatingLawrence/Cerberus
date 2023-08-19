#include "cerberusobject.h"

#include "src/core/cerberusfactory.h"
#include "src/define.h"

using namespace cerberus;

//=============================================================================
std::string CerberusObject::toStr(const CerberusObject& obj)
{
    std::string ret;

    switch (obj.m_type)
    {
        case Thread:
            ret.append("Thread");
            break;
        case MessageTemplate:
            ret.append("Template");
            break;
        case Socket:
            ret.append("Socket");
            break;
        default:
            ret.append("Unknown type");
            break;
    }

    switch (obj.m_socketType)
    {
        case Socket_None:
            break;
        case Socket_UDP:
            ret.append(" UDP");
            break;
        case Socket_TCP:
            ret.append(" TCP");
            break;
        case Socket_TCPP2P:
            ret.append(" TCP P2P");
            break;
        case Socket_HTTP:
            ret.append(" HTTP");
            break;
        case Socket_WEB:
            ret.append(" WEB");
            break;
        case Socket_ICMP:
            ret.append(" ICMP");
            break;
        case Socket_IPC:
            ret.append(" IPC");
            break;
    }

    ret.append(", ID:");

    if (obj.m_id == CERBERUS_INVALID_ID)
    {
        ret.append("INVALID");
    }
    else
    {
        ret.append(core::CerberusUtils::strPrint("%lx", obj.m_id));
    }

    if (!obj.m_name.empty())
    {
        ret.append(", NAME:");
        ret.append(obj.m_name);
    }

    return ret;
}
//=============================================================================
std::string CerberusObject::toObjStr() { return CerberusObject::toStr(*this); }
//=============================================================================
CerberusObject::CerberusObject(ObjectType type, const std::string& name)
    : m_id(Socket_None),
      m_type(type),
      m_name(name),
      m_socketType(Socket_None)
{
    m_id = core::CerberusFactory::_registerCerberusObject(this);
}
//=============================================================================
CerberusObject::CerberusObject(SocketType type, const std::string& name)
    : m_id(CERBERUS_INVALID_ID),
      m_type(Socket),
      m_name(name),
      m_socketType(type)
{
    if (m_socketType != Socket_None) m_id = core::CerberusFactory::_registerCerberusObject(this);
}
//=============================================================================
CerberusObject::~CerberusObject() { core::CerberusFactory::_unregisterCerberusObject(m_id); }
//=============================================================================
bool CerberusObject::isObjValid() { return (m_id != CERBERUS_INVALID_ID); }
//=============================================================================
uint32_t CerberusObject::id() const { return m_id; }
//=============================================================================
CerberusObject::ObjectType CerberusObject::type() const { return m_type; }
//=============================================================================
CerberusObject::SocketType CerberusObject::socketType() const { return m_socketType; }
//=============================================================================
std::string CerberusObject::name() const { return m_name; }
//=============================================================================
