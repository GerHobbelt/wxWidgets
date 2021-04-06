
#pragma once

#include "pch.h"

#include "db_database.h"
#include "db_introspector.h"
#include "db_object.h"
#include "db_relationship.h"
#include "db_vector.h"

struct cDbTraits {
  template <typename T>
#if 1
  using alloc = std::allocator<T>;
#else
  using alloc = shm::alloc<T>;
#endif

  using uid_t = int;

  enum class eObjId { Comp, Pin, ModelSelector, _count };
  enum class ePropId { Comp_Name, Pin_Name, ModelSelector_Name, _count };
  enum class eRelId { Comp_PowerPin, Comp_Pin, Comp_ModelSelector, _count };
  using cIntrospector = db::cIntrospector<cDbTraits>;
  static cIntrospector introspector;
};
