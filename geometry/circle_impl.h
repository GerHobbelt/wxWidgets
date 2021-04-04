#pragma once

#include "geom_impl_base.h"

struct cCircleImpl
   : public cGeomImplBase
{
   cCircle m_circ;

   cCircleImpl(bool hole, bool filled, coord_t center_x, coord_t center_y, coord_t radius TAG)
      : cGeomImplBase(iPolygon::Type::circle, hole, filled PASS_TAG)
      , m_circ(center_x, center_y, radius)
   {
   }

   double length() const
   {
      return cavc::utils::tau<coord_t>() * m_circ.m_radius;
   }
   double area() const
   {
      return cavc::utils::pi<coord_t>() * m_circ.m_radius * m_circ.m_radius;
   }
   cRect rectangle() const
   {
      return m_circ.rectangle();
   }
   cCircle circle() const
   {
      return m_circ;
   }
};
