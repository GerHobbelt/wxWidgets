
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
  enum class eObjId { Comp, Pin, _count };
  enum class ePropId { Comp_Name, Pin_Name, _count };
  enum class eRelId { Comp_Pin, _count };
  static db::cIntrospector<cDbTraits> introspector;
};
