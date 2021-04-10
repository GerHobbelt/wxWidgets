#pragma once

#include "geom_impl_base.h"

struct cCircleImpl
   : public cGeomImplBase
{
   geom::cCircle m_circ;

   cCircleImpl(bool hole, bool filled, geom::coord_t center_x, geom::coord_t center_y, geom::coord_t radius TAG)
      : cGeomImplBase(geom::iPolygon::Type::circle, hole, filled PASS_TAG)
      , m_circ(center_x, center_y, radius)
   {
   }

   double length() const
   {
      return 2 * geom::pi() * m_circ.m_radius;
   }
   double area() const
   {
      return geom::pi() * m_circ.m_radius * m_circ.m_radius;
   }
   geom::cRect rectangle() const
   {
      return m_circ.rectangle();
   }
   geom::cCircle circle() const
   {
      return m_circ;
   }
};
