#pragma once

#include "geom_impl_base.h"

struct cArcSegmentImpl
   : public cGeomImplBase
{
   geom::cArc m_seg;

   cArcSegmentImpl(bool hole, bool filled, const geom::cPoint& beg, const geom::cPoint& end, const geom::cPoint& center, geom::coord_t r, geom::coord_t width TAG)
      : cGeomImplBase(geom::iPolygon::Type::arc_segment, hole, filled PASS_TAG)
      , m_seg(beg, end, center, r, width / 2)
   {
   }
   cArcSegmentImpl(bool hole, bool filled, const geom::cPoint& beg, const geom::cPoint& end, geom::coord_t bulge, geom::coord_t width TAG)
      : cGeomImplBase(geom::iPolygon::Type::arc_segment, hole, filled PASS_TAG)
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
   geom::cRect rectangle() const
   {
      return m_seg.rectangle();
   }

   geom::cSegment segment() const
   {
      return m_seg;
   }

   geom::cArc arc_segment() const
   {
      return m_seg;
   }
};
