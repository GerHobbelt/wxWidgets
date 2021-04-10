#pragma once

#include "geom_impl_base.h"

struct cRectImpl
   : public cGeomImplBase
{
   geom::cRect m_rect;

   cRectImpl(bool hole, bool filled, geom::coord_t left, geom::coord_t bottom, geom::coord_t right, geom::coord_t top TAG)
      : cGeomImplBase(geom::iPolygon::Type::rectangle, hole, filled PASS_TAG)
      , m_rect(left, bottom, right, top)
   {
   }

   double area() const
   {
      return m_rect.area();
   }
   geom::cRect rectangle() const
   {
      return m_rect;
   }
};
