
#pragma once

#include "database_traits.h"

class cComp;

class cPin : public db::cObject<cDbTraits> {
public:
  // property Name
  std::string m_Name;

public:
  cPin() : cObject(cDbTraits::eObjId::Pin) {}
  ~cPin() { on_destroy(); }

  // property Name
  std::string getName() const { return m_Name; }
  void setName(const std::string &val) {
    if (m_Name != val) {
      before_propmodify(cDbTraits::ePropId::Pin_Name, m_Name);
      m_Name = val;
      after_propmodify(cDbTraits::ePropId::Pin_Name, m_Name);
    }
  }

  // relationship Pin->Comp
  cComp *parentComp() const {
    return (cComp *)parent(cDbTraits::eRelId::Comp_Pin);
  }
  size_t countComp() const { return count(cDbTraits::eRelId::Comp_Pin, true); }
};
