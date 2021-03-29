#pragma once

#include "geom_impl.h"

struct cRectImpl
   : public cGeomImpl
{
   cRect m_rect;

   cRectImpl(bool hole, bool filled, double left, double bottom, double right, double top TAG)
      : cGeomImpl(hole, filled PASS_TAG)
      , m_rect(left, bottom, right, top)
   {
   }

   Type type() const override
   {
      return Type::rectangle;
   }

   double area() const override
   {
      return m_rect.area();
   }
   cRect rectangle() const override
   {
      return m_rect;
   }
};
