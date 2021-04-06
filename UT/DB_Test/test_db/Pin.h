
#pragma once

#include "database_traits.h"

class cComp;

class cPin : public db::cObject<cDbTraits> {
public:
  // property Name
  std::string m_Name;

public:
  cPin() : cObject(cDbTraits::eObjId::Pin) {}
  ~cPin() {}

  // property Name
  std::string getName() const { return m_Name; }
  void setName(const std::string &val) {
    if (m_Name != val) {
      before_propmodify(cDbTraits::ePropId::Pin_Name, m_Name);
      m_Name = val;
      after_propmodify(cDbTraits::ePropId::Pin_Name, m_Name);
    }
  }

  // relationship (One2One) Comp->PowerPin
  void includePPinComp(cComp &x) {
    include(cDbTraits::eRelId::Comp_PowerPin, (db::cObject<cDbTraits> &)x);
  }
  cComp *parentPPinComp() const {
    return (cComp *)parent(cDbTraits::eRelId::Comp_PowerPin);
  }
  size_t countPPinComps() const {
    return count(cDbTraits::eRelId::Comp_PowerPin);
  }

  // relationship (One2Many) Comp->Pin
  void includeComp(cComp &x) {
    include(cDbTraits::eRelId::Comp_Pin, (db::cObject<cDbTraits> &)x);
  }
  cComp *parentComp() const {
    return (cComp *)parent(cDbTraits::eRelId::Comp_Pin);
  }
  size_t countComps() const { return count(cDbTraits::eRelId::Comp_Pin); }
};
