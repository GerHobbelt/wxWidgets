// dbtest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <pch.h>

#include "db_vector.h"
#include "db_object.h"
#include "db_relationship.h"
#include "db_introspector.h"
#include "db_database.h"

using namespace std;

struct cTestDbTraits
{
   template <typename T>
#if 1
   using alloc = allocator<T>;
#else
   using alloc = shm::alloc<T>;
#endif

   using uid_t = int;

   enum class eObjId {
      Net,
      Comp,
      Pin,
      PinPair,
      Pad,

      _count
   };

   enum class ePropId {
      Comp_Name,
      Pin_Name,

      _count
   };

   enum class eRelId {
      Comp_Pin,
      Comp_PinPair,
      Net_Pin,
      Pin_Pad,
      Comp_PowerPin,

      _count
   };

   static db::cIntrospector<cTestDbTraits> introspector;

   struct cPin : public db::cObject<cTestDbTraits>
   {
      string m_Name;

      cPin(const char* s = nullptr)
         : cObject(eObjId::Pin)
         , m_Name(s)
      {
      }
   };

   struct cComp : public db::cObject<cTestDbTraits>
   {
      string m_Name;

      cComp(const char* s = nullptr)
         : cObject(eObjId::Comp)
         , m_Name(s)
      {
      }
      cComp(cComp&& x)
         : cObject(move(x))
      {
      }

      void include_pin(cPin& pin)
      {
         include(eRelId::Comp_Pin, pin);
      }
      void exclude_pin(cPin& pin)
      {
         exclude(eRelId::Comp_Pin, pin);
      }
   };
};

using cObject = db::cObject<cTestDbTraits>;
using cIntrospector = db::cIntrospector<cTestDbTraits>;
using ePropertyType = typename cIntrospector::ePropertyType;
using eRelationshipType = typename cIntrospector::eRelationshipType;

//auto x = &cObject::factory<cTestDbTraits::cComp>;
//auto y = cIntrospector::cObjDesc::factory_t();
////cIntrospector::cObjDesc::factory_t z = &cObject::factory<cTestDbTraits::cComp>;
//cIntrospector::pointer<cObject> a = cObject::alloc_traits<cObject>::pointer();

#define OBJ_DESC(id) cIntrospector::cObjDesc{#id, eObjId::##id, &cObject::factory<c##id>}
#define PROP_DESC(id, type, proptype) cIntrospector::cPropDesc{#id, ePropId::##type##_##id, ePropertyType::proptype, (intptr_t)&((c##type##*)0)->m_##id}
#define REL_DESC(id, type, parent, child) cIntrospector::cRelDesc{#id, eRelationshipType::type, eRelId::##id, eObjId::parent, eObjId::child}

cIntrospector cTestDbTraits::introspector = {
   {
//      OBJ_DESC(Net),
      OBJ_DESC(Comp),
      OBJ_DESC(Pin),
//      OBJ_DESC(PinPair),
//      OBJ_DESC(Pad)
   },{
      PROP_DESC(Name, Comp, string),
      PROP_DESC(Name, Pin, string)
   },{
      REL_DESC(Comp_Pin, One2Many, Comp, Pin),
      REL_DESC(Net_Pin, One2Many, Net, Pin),
      REL_DESC(Pin_Pad, One2Many, Pin, Pad),
      REL_DESC(Comp_PowerPin, One2One, Comp, Pin)
   }
};

#undef REL_DESC
#undef PROP_DESC
#undef OBJ_DESC

int main()
{
   cTestDbTraits::cPin p1("U1.1"), p2("U1.2"), p3("U1.3");
   cTestDbTraits::cComp u1("U1"), u2("U2");

   u1.include_pin(p1);
   u1.include_pin(p2);
   u1.exclude_pin(p1);
   u1.include_pin(p3);
   u1.include(cTestDbTraits::eRelId::Comp_PowerPin, p1);
}
