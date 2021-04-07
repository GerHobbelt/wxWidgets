
#pragma once

#include "database_traits.h"

#include "ModelSelector.h"
#include "Pin.h"
#include "db_string.h"

class cComp : public db::cObject<cDbTraits> {
public:
  // property Name
  db::string<char> m_Name;

public:
  cComp() : cObject(cDbTraits::eObjId::Comp) {}
  ~cComp() {}

  // property Name
  const char *getName() const { return m_Name.c_str(); }
  void setName(const char *val) {
    if (m_Name != val) {
      before_propmodify(cDbTraits::ePropId::Comp_Name, m_Name);
      m_Name = val;
      after_propmodify(cDbTraits::ePropId::Comp_Name, m_Name);
    }
  }

  // relationship (One2One) Comp->PowerPin
  void includePowerPin(cPin &x) {
    include(cDbTraits::eRelId::Comp_PowerPin, x);
  }
  void removePowerPin() { exclude(cDbTraits::eRelId::Comp_PowerPin); }
  size_t countPowerPins() const {
    return count(cDbTraits::eRelId::Comp_PowerPin);
  }

  // relationship (One2Many) Comp->Pin
  void includePin(cPin &x) { include(cDbTraits::eRelId::Comp_Pin, x); }
  void excludePin(cPin &x) { exclude(cDbTraits::eRelId::Comp_Pin, x); }
  auto Pins() {
    return db::cRelIterRange<cDbTraits, cComp, cPin>(
        this, cDbTraits::eRelId::Comp_Pin);
  }
  auto Pins() const {
    return db::cRelIterConstRange<cDbTraits, cComp, cPin>(
        this, cDbTraits::eRelId::Comp_Pin);
  }
  size_t countPins() const { return count(cDbTraits::eRelId::Comp_Pin); }

  // relationship (Many2Many) Comp->ModelSelector
  void includeModelSelector(cModelSelector &x) {
    include(cDbTraits::eRelId::Comp_ModelSelector, x);
  }
  void excludeModelSelector(cModelSelector &x) {
    exclude(cDbTraits::eRelId::Comp_ModelSelector, x);
  }
  auto ModelSelectors() {
    return db::cRelIterRange<cDbTraits, cComp, cModelSelector>(
        this, cDbTraits::eRelId::Comp_ModelSelector);
  }
  auto ModelSelectors() const {
    return db::cRelIterConstRange<cDbTraits, cComp, cModelSelector>(
        this, cDbTraits::eRelId::Comp_ModelSelector);
  }
  size_t countModelSelectors() const {
    return count(cDbTraits::eRelId::Comp_ModelSelector);
  }
};
