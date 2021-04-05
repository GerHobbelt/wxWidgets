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
   iPolygon::Type m_type;
   bool m_hole, m_filled, m_static = false;

   union {
      unsigned m_holes_count = -1; // for outlines
      unsigned m_outline_idx; // for holes
   };

#ifdef USE_TAG
   shm::string m_tag;
#endif

   cGeomImplBase(iPolygon::Type type, bool hole, bool filled TAG)
      : m_type(type)
      , m_hole(hole)
      , m_filled(filled)
#ifdef USE_TAG
      , m_tag(tag ? tag : "")
#endif
   {
   }
};
