
#pragma once

#include "database_traits.h"

#include "db_string.h"
class cComp;

class cPin : public db::cObject<cDbTraits> {
public:
  // property Name
  db::string<char> m_Name;

public:
  cPin() : cObject(cDbTraits::eObjId::Pin) {}
  ~cPin() {}

  // property Name
  const char *getName() const { return m_Name.c_str(); }
  void setName(const char *val) {
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
