
#pragma once

using eObjId = typename cDbTraits::eObjId;
using ePropId = typename cDbTraits::ePropId;
using eRelId = typename cDbTraits::eRelId;

using cObject = db::cObject<cDbTraits>;
using cRelationship = db::cRelationship<cDbTraits>;
using cIntrospector = db::cIntrospector<cDbTraits>;
using ePropertyType = typename cIntrospector::ePropertyType;
using eRelationshipType = typename cIntrospector::eRelationshipType;
