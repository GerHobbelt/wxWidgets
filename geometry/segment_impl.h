#pragma once

#include "geom_impl_base.h"

struct cSegmentImpl
   : public cGeomImplBase
{
   cSegment m_seg;

   cSegmentImpl(bool hole, bool filled, const cPoint& beg, const cPoint& end, coord_t width TAG)
      : cGeomImplBase(iPolygon::Type::segment, hole, filled PASS_TAG)
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
   cRect rectangle() const
   {
      return m_seg.rectangle();
   }

   cSegment segment() const
   {
      return m_seg;
   }
};
