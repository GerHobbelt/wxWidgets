
#pragma once

#include "Comp.h"

#include "Pin.h"

#include "ModelSelector.h"

class cDatabase : public db::cDatabase<cDbTraits> {
public:
  auto createComp() { return (cComp *)create(eObjId::Comp); }
  auto Comps() {
    return typename db::cDatabase<cDbTraits>::iterator_range<cComp>(
        this, eObjId::Comp);
  }
  auto Comps() const {
    return typename db::cDatabase<cDbTraits>::const_iterator_range<cComp>(
        this, eObjId::Comp);
  }
  auto createPin() { return (cPin *)create(eObjId::Pin); }
  auto Pins() {
    return typename db::cDatabase<cDbTraits>::iterator_range<cPin>(this,
                                                                   eObjId::Pin);
  }
  auto Pins() const {
    return typename db::cDatabase<cDbTraits>::const_iterator_range<cPin>(
        this, eObjId::Pin);
  }
  auto createModelSelector() {
    return (cModelSelector *)create(eObjId::ModelSelector);
  }
  auto ModelSelectors() {
    return typename db::cDatabase<cDbTraits>::iterator_range<cModelSelector>(
        this, eObjId::ModelSelector);
  }
  auto ModelSelectors() const {
    return
        typename db::cDatabase<cDbTraits>::const_iterator_range<cModelSelector>(
            this, eObjId::ModelSelector);
  }
};
