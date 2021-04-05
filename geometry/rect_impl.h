#pragma once

#include "geom_impl_base.h"

struct cRectImpl
   : public cGeomImplBase
{
   cRect m_rect;

   cRectImpl(bool hole, bool filled, double left, double bottom, double right, double top TAG)
      : cGeomImplBase(iPolygon::Type::rectangle, hole, filled PASS_TAG)
      , m_rect(left, bottom, right, top)
   {
   }

   double area() const
   {
      return m_rect.area();
   }
   cRect rectangle() const
   {
      return m_rect;
   }
};
