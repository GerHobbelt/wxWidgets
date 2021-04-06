
#pragma once

#include "database_traits.h"

#include "Pin.h"

class cComp : public db::cObject<cDbTraits> {
public:
  // property Name
  std::string m_Name;

public:
  cComp() : cObject(cDbTraits::eObjId::Comp) {}
  ~cComp() { on_destroy(); }

  // property Name
  std::string getName() const { return m_Name; }
  void setName(const std::string &val) {
    if (m_Name != val) {
      before_propmodify(cDbTraits::ePropId::Comp_Name, m_Name);
      m_Name = val;
      after_propmodify(cDbTraits::ePropId::Comp_Name, m_Name);
    }
  }

  // relationship Comp->Pin
  void includePin(cPin &x) { include(cDbTraits::eRelId::Comp_Pin, x); }
  void excludePin(cPin &x) { exclude(cDbTraits::eRelId::Comp_Pin, x); }
  size_t countPin() const { return count(cDbTraits::eRelId::Comp_Pin, false); }
};
