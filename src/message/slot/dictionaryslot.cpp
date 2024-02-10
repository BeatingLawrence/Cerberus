#include "dictionaryslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot DictionarySlot::create(const Dictionary& value) { return cerberus_slot(new DictionarySlot(value)); }
//=============================================================================
cerberus::cerberus_slot DictionarySlot::create(const DictionarySlot& other) { return cerberus_slot(new DictionarySlot(other)); }
//=============================================================================
DictionarySlot::DictionarySlot(const Dictionary& value)
    : BaseSlot(ST_DICTIONARY),
      m_dict(value)
{
    // noop
}
//=============================================================================
const cerberus::Dictionary& DictionarySlot::value() const { return m_dict; }
//=============================================================================
cerberus::Dictionary& DictionarySlot::value() { return m_dict; }
//=============================================================================
void DictionarySlot::value(const Dictionary& value) { m_dict.assign(value.begin(), value.end()); }
//=============================================================================
