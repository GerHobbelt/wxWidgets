#pragma once

#include "geom_impl_base.h"

struct cArcSegmentImpl
   : public cGeomImplBase
{
   cArc m_seg;

   cArcSegmentImpl(bool hole, bool filled, const cPoint& beg, const cPoint& end, const cPoint& center, coord_t r, coord_t width TAG)
      : cGeomImplBase(iPolygon::Type::arc_segment, hole, filled PASS_TAG)
      , m_seg(beg, end, center, r, width / 2)
   {
   }
   cArcSegmentImpl(bool hole, bool filled, const cPoint& beg, const cPoint& end, coord_t bulge, coord_t width TAG)
      : cGeomImplBase(iPolygon::Type::arc_segment, hole, filled PASS_TAG)
      , m_seg(beg, end, bulge, width / 2)
   {
   }

   double length() const
   {
      return m_seg.length();
   }
   double area() const
   {
      return m_seg.area();
   }
   cRect rectangle() const
   {
      return m_seg.rectangle();
   }

   cSegment segment() const
   {
      return m_seg;
   }

   cArc arc_segment() const
   {
      return m_seg;
   }
};
