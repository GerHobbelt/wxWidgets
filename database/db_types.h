#pragma once

#include "db_introspector.h"

namespace db {

   template <typename Traits>
   struct cTypes
   {
   protected:
      template <class T>
      using alloc = typename Traits::template alloc<T>;

      template <class T>
      using alloc_traits = allocator_traits<alloc<T>>;

      using cIntrospector = decltype(Traits::introspector);

      using cObjDescs = typename decltype(cIntrospector::m_obj_desc);
      using cObjDesc = typename cObjDescs::value_type;

      using cPropDescs = typename decltype(cIntrospector::m_prop_desc);
      using cPropDesc = typename cPropDescs::value_type;

      using cRelDescs = typename decltype(cIntrospector::m_rel_desc);
      using cRelDesc = typename cRelDescs::value_type;

      using eObjId = decltype(cObjDesc::m_id);
      using ePropId = decltype(cPropDesc::m_id);
      using eRelId = decltype(cRelDesc::m_id);

      using uid_t = typename Traits::uid_t;

      using eRelationshipType = typename cIntrospector::eRelationshipType;

      using cObject = cObject<Traits>;
      using cObjectPtr = typename alloc_traits<cObject>::pointer;
   };

} // namespace db
