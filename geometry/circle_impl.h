#pragma once

#include "geom_impl.h"

struct cCircleImpl
   : public cGeomImpl
{
   cCircle m_circ;

   cCircleImpl(bool hole, bool filled, coord_t center_x, coord_t center_y, coord_t radius TAG)
      : cGeomImpl(hole, filled PASS_TAG)
      , m_circ(center_x, center_y, radius)
   {
   }

   Type type() const override
   {
      return Type::circle;
   }

   double length() const override
   {
      return cavc::utils::tau<coord_t>() * m_circ.m_radius;
   }
   double area() const override
   {
      return cavc::utils::pi<coord_t>() * m_circ.m_radius * m_circ.m_radius;
   }
   cRect rectangle() const override
   {
      return m_circ.rectangle();
   }
   cCircle circle() const override
   {
      return m_circ;
   }
};
