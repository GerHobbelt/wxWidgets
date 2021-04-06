#pragma once

#include <array>

namespace db {

#include "db_object.h"
#include "db_relationship.h"

using namespace std;

template <typename Traits>
class cDatabase
{
protected:
   using cIntrospector = cIntrospector<Traits>;

   using cObjDescs = decltype(cIntrospector::m_obj_desc);
   using cObjDesc = typename cObjDescs::value_type;

   using cPropDescs = decltype(cIntrospector::m_prop_desc);;
   using cPropDesc = typename cPropDescs::value_type;

   using cRelDescs = decltype(cIntrospector::m_rel_desc);;
   using cRelDesc = typename cRelDescs::value_type;

   using eRelationshipType = typename cIntrospector::eRelationshipType;

public:
   using cObject = cObject<Traits>;
   using cObjectPtr = cObjectPtr<Traits>;
   using cRelationship = cRelationship<Traits>;

   using eObjId = typename cIntrospector::eObjId;
   using ePropId = typename cIntrospector::ePropId;
   using eRelId = typename cIntrospector::eRelId;

   cObjectPtr create(eObjId id)
   {
      cObjectPtr new_obj = nullptr;
      if (cObjDesc* objdesc = Traits::introspector.find_obj_desc(id)) {
         if (objdesc->m_factory) {
            new_obj = objdesc->m_factory();
            auto& obj_list = m_objects[size_t(id)];
            obj_list.push_back(*new_obj);
         }
      }
      return new_obj;
   }

protected:
   //array<cRelationship, Traits::nObjTypes> m_objects;
   array<bin::list<cObject>, size_t(eObjId::_count)> m_objects;
};

} // namespace db
