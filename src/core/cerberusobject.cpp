#include "cerberusobject.h"

#include "src/cerberus.h"
#include "src/define.h"
#include "src/thread/thread.h"

using namespace cerberus::core;

//=============================================================================
std::string CerberusObject::toStr(const CerberusObject& obj)
{
    std::string ret;

    switch (obj.m_type)
    {
        case InvalidObject:
            return "Invalid object";
        case Thread:
            ret.append("Thread");
            break;
        case MessageTemplate:
            ret.append("Message template");
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

    if (obj.m_type == Thread)
    {
        ret.append(", ");
        ret.append(toThreadStr(obj));
    }

    return ret;
}
//=============================================================================
std::string CerberusObject::toObjStr() { return CerberusObject::toStr(*this); }
//=============================================================================
std::string CerberusObject::toThreadStr(const CerberusObject& obj)
{
    const thread::Thread* thr = obj.to_p<cerberus::thread::Thread>();
    auto time                 = thr->getTime();

    switch (thr->getPeriodicity())
    {
        case TP_NonPeriodic:
            return "Configuration: Non periodic";
        case TP_Periodic:
            return CerberusUtils::strPrint("Configuration: Periodic, %us %uns", time.seconds,
                                           time.nanoseconds);
        case TP_PeriodicQueue:
            return CerberusUtils::strPrint("Configuration: Periodic queue, %us %uns", time.seconds,
                                           time.nanoseconds);
        case TP_OneShot:
            return "Configuration: One shot";
        case TP_Continuos:
            return "Configuration: Continuos";
    }

    return "";
}
//=============================================================================
CerberusObject::CerberusObject(ObjectType type, const std::string& name)
    : m_id(CERBERUS_INVALID_ID),
      m_type(type),
      m_name(name),
      m_socketType(Socket_None)
{
}
//=============================================================================
CerberusObject::CerberusObject(SocketType type, const std::string& name)
    : m_id(CERBERUS_INVALID_ID),
      m_type(Socket),
      m_name(name),
      m_socketType(type)
{
}
//=============================================================================
CerberusObject::~CerberusObject() {}
//=============================================================================
void CerberusObject::checkIn() { cerberus::Cerberus::registerObj(this); }
//=============================================================================
void CerberusObject::checkOut() { cerberus::Cerberus::unregisterObj(m_id); }
//=============================================================================
bool CerberusObject::isObjValid() const { return (m_id != CERBERUS_INVALID_ID); }
//=============================================================================
cerberus::HASH32 CerberusObject::id() const { return m_id; }
//=============================================================================
CerberusObject::ObjectType CerberusObject::type() const { return m_type; }
//=============================================================================
CerberusObject::SocketType CerberusObject::socketType() const { return m_socketType; }
//=============================================================================
std::string CerberusObject::name() const { return m_name; }
//=============================================================================
