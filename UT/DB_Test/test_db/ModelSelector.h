
#pragma once

#include "database_traits.h"

class cComp;

class cModelSelector : public db::cObject<cDbTraits> {
public:
  // property Name
  std::string m_Name;

public:
  cModelSelector() : cObject(cDbTraits::eObjId::ModelSelector) {}
  ~cModelSelector() {}

  // property Name
  std::string getName() const { return m_Name; }
  void setName(const std::string &val) {
    if (m_Name != val) {
      before_propmodify(cDbTraits::ePropId::ModelSelector_Name, m_Name);
      m_Name = val;
      after_propmodify(cDbTraits::ePropId::ModelSelector_Name, m_Name);
    }
  }

  // relationship (Many2Many) Comp->ModelSelector
  void includeComp(cComp &x) {
    include(cDbTraits::eRelId::Comp_ModelSelector, (db::cObject<cDbTraits> &)x);
  }
  void excludeComp(cModelSelector &x) {
    exclude(cDbTraits::eRelId::Comp_ModelSelector, x);
  }
  auto ModelSelectors() {
    return db::cRelIterRange<cDbTraits, cModelSelector, cModelSelector>(
        this, cDbTraits::eRelId::Comp_ModelSelector);
  }
  auto ModelSelectors() const {
    return db::cRelIterConstRange<cDbTraits, cModelSelector, cModelSelector>(
        this, cDbTraits::eRelId::Comp_ModelSelector);
  }
  size_t countComps() const {
    return count(cDbTraits::eRelId::Comp_ModelSelector);
  }
};
