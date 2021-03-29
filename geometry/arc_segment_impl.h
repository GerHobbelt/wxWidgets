#pragma once

#include "geom_impl.h"

struct cArcSegmentImpl
   : public cGeomImpl
{
   cArc m_seg;

   cArcSegmentImpl(bool hole, bool filled, const cPoint& beg, const cPoint& end, const cPoint& center, coord_t r, bool ccw, coord_t width TAG)
      : cGeomImpl(hole, filled PASS_TAG)
      , m_seg(beg, end, center, ccw ? r : -r, width / 2)
   {
   }

   Type type() const override
   {
      return Type::arc_segment;
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
      return m_seg.cCircle::rectangle(); //TBD
   }

   cSegment segment() const override
   {
      return m_seg;
   }

   cArc arc_segment() const override
   {
      return m_seg;
   }
};
