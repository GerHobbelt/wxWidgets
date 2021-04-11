#pragma once

#include "geom_model.h"
#include "shared_memory.h"

//#define USE_TAG
#ifdef USE_TAG
#define TAG , const char* tag
#define PASS_TAG , tag
#else
#define TAG
#define PASS_TAG
#endif

struct cGeomImplBase
{
   geom::iPolygon::Type m_geom_type;
   bool m_hole : 1, m_filled : 1;
   intptr_t m_object_offset : 54;

#ifdef USE_TAG
   shm::string m_tag;
#endif

   cGeomImplBase(geom::iPolygon::Type type, bool hole, bool filled TAG)
      : m_geom_type(type)
      , m_hole(hole)
      , m_filled(filled)
#ifdef USE_TAG
      , m_tag(tag ? tag : "")
#endif
   {
   }

   void* object()
   {
      return (char*)this + m_object_offset;
   }
   void set_object(void* obj)
   {
      m_object_offset = intptr_t((char*)obj - (char*)this);
   }
};

struct cGeomImplGroup
   : public cGeomImplBase
   , public shm::vector<cGeomImplBase*>
{
   cGeomImplGroup()
      : cGeomImplBase(geom::iPolygon::Type::group, false, false)
   {
   }
};
