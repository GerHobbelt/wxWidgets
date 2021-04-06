
#include "pch.h"

#include "database.h"

using namespace std;

#include "database_traits_types.h"

#define OBJ_DESC(id)                                                           \
  cIntrospector::cObjDesc {                                                    \
#id, eObjId::##id, &cObject::factory < c##id>, &cObject::disposer < c##id> \
  }

#define PROP_DESC(id, type, proptype)                                          \
  cIntrospector::cPropDesc {                                                   \
#id, ePropId::##type##_##id, ePropertyType::proptype,                      \
        (intptr_t) & ((c##type##*)0)->m_##id                                   \
  }

#define REL_DESC(id, type, parent, child)                                      \
  cIntrospector::cRelDesc {                                                    \
#id, eRelationshipType::type, eRelId::##id, eObjId::parent, eObjId::child  \
  }

cIntrospector cDbTraits::introspector = {
    {
        OBJ_DESC(Comp),
        OBJ_DESC(Pin),
        OBJ_DESC(ModelSelector),
    },
    {
        PROP_DESC(Name, Comp, string),
        PROP_DESC(Name, Pin, string),
        PROP_DESC(Name, ModelSelector, string),
    },
    {
        REL_DESC(Comp_PowerPin, One2One, Comp, Pin),
        REL_DESC(Comp_Pin, One2Many, Comp, Pin),
        REL_DESC(Comp_ModelSelector, Many2Many, Comp, ModelSelector),
    }};

#undef REL_DESC
#undef PROP_DESC
#undef OBJ_DESC
