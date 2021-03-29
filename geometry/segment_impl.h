#pragma once

#include "geom_impl.h"

struct cSegmentImpl
   : public cGeomImpl
{
   cSegment m_seg;

   cSegmentImpl(bool hole, bool filled, const cPoint& beg, const cPoint& end, coord_t width TAG)
      : cGeomImpl(hole, filled PASS_TAG)
      , m_seg(beg, end, width / 2)
   {
   }

   Type type() const override
   {
      return Type::segment;
   }

   double length() const override
   {
      return m_seg.length();
   }
   double area() const override
   {
      return m_seg.area();
   }
   cRect rectangle() const override
   {
      return m_seg.rectangle();
   }

   cSegment segment() const override
   {
      return m_seg;
   }
};
