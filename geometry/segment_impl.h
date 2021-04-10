#pragma once

#include "geom_impl_base.h"

struct cSegmentImpl
   : public cGeomImplBase
{
   geom::cSegment m_seg;

   cSegmentImpl(bool hole, bool filled, const geom::cPoint& beg, const geom::cPoint& end, geom::coord_t width TAG)
      : cGeomImplBase(geom::iPolygon::Type::segment, hole, filled PASS_TAG)
      , m_seg(beg, end, width / 2)
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
};
