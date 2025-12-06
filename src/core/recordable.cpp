#include "recordable.h"

#include "src/cerberus.h"
#include "src/define.h"
#include "src/thread/thread.h"

using namespace cerberus::core;

//=============================================================================
std::string Recordable::toStr(const Recordable& obj)
{
    std::string ret;

    ret.append("ID:");

    if (obj.m_id == CERBERUS_INVALID_ID)
        ret.append("INVALID");
    else
        ret.append(CerberusUtils::strPrint("%lx", obj.m_id));

    if (!obj.m_name.empty())
    {
        ret.append(", NAME:");
        ret.append(obj.m_name);
    }

    return ret;
}
//=============================================================================
std::string Recordable::toObjStr() { return Recordable::toStr(*this); }
//=============================================================================
std::string Recordable::toThreadStr(const Recordable& obj)
{
    auto thr  = obj.to_p<const cerberus::Thread>();
    auto time = thr->getTime();

    switch (thr->getPeriodicity())
    {
        case TP_Message:
            return "Configuration: Non periodic";

        case TP_Periodic:
            return CerberusUtils::strPrint("Configuration: Periodic, %us %uns", time.seconds,
                                           time.nanoseconds);

        case TP_PeriodicMessage:
            return CerberusUtils::strPrint("Configuration: Periodic-message, %us %uns", time.seconds,
                                           time.nanoseconds);

        case TP_OneShot:
            return "Configuration: One shot";

        case TP_Continuos:
            return "Configuration: Continuos";

        case TP_Trigger:
            return "Configuration: Trigger";
    }

    return "";
}
//=============================================================================
Recordable::Recordable(const std::string& name)
    : m_id(CERBERUS_INVALID_ID),
      m_name(name)
{
}
//=============================================================================
Recordable::~Recordable() {}
//=============================================================================
void Recordable::checkIn() { cerberus::Cerberus::registerObj(this); }
//=============================================================================
void Recordable::checkOut() { cerberus::Cerberus::unregisterObj(m_id); }
//=============================================================================
bool Recordable::isRegistered() const { return (m_id != CERBERUS_INVALID_ID); }
//=============================================================================
cerberus::HASH32 Recordable::id() const { return m_id; }
//=============================================================================
std::string Recordable::name() const { return m_name; }
//=============================================================================
