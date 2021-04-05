#pragma once

#include "circle_impl.h"
#include "segment_impl.h"
#include "arc_segment_impl.h"
#include "shape_impl.h"
#include "rect_impl.h"

struct cGeomImpl
   : public iGeomImpl
{
   cGeomImplBase* m_pGeom = nullptr;

   using holes_t = list<iGeomImpl*>;
   shared_ptr<holes_t> m_holes;

   cGeomImpl(cGeomImplBase* pGeom)
      : m_pGeom(pGeom)
      , m_holes(make_shared<holes_t>())
   {
   }

   cGeomImplBase* geom_data() override
   {
      return m_pGeom;
   }
   void set_geom_data(cGeomImplBase* pGeom) override
   {
      m_pGeom = pGeom;
   }
   list<iGeomImpl*>& holes() override
   {
      return *m_holes;
   }

   Type type() const override
   {
      return m_pGeom->m_type;
   }

   bool empty() const override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            return ((cShapeImpl*)m_pGeom)->empty();
      }
      return false;
   }
   bool closed() const override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            return ((cShapeImpl*)m_pGeom)->closed();
      }
      return true;
   }
   bool filled() const override
   {
      return m_pGeom->m_filled;
   }
   bool hole() const override
   {
      return m_pGeom->m_hole;
   }

   cVertexIter vertices() const override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            return ((cShapeImpl*)m_pGeom)->vertices();
      }
      return cVertexIter();
   }
   bool outline(iPolygon** res) const override
   {
      return false;
   }
   void holes(iPolygonIter** res) const override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            ((cShapeImpl*)m_pGeom)->holes(res);
            break;
      }
   }

   cSegment segment() const override
   {
      switch (type()) {
         case iPolygon::Type::segment:
            return ((cSegmentImpl*)m_pGeom)->segment();
         case iPolygon::Type::arc_segment:
            return ((cArcSegmentImpl*)m_pGeom)->segment();
      }
      return cSegment();
   }
   cArc arc_segment() const override
   {
      switch (type()) {
         case iPolygon::Type::arc_segment:
            return ((cArcSegmentImpl*)m_pGeom)->arc_segment();
      }
      return cArc();
   }
   cCircle circle() const override
   {
      switch (type()) {
         case iPolygon::Type::circle:
            return ((cCircleImpl*)m_pGeom)->circle();
      }
      return cCircle();
   }
   cRect rectangle() const override
   {
      switch (type()) {
         case iPolygon::Type::circle:
            return ((cCircleImpl*)m_pGeom)->rectangle();
         case iPolygon::Type::segment:
            return ((cSegmentImpl*)m_pGeom)->rectangle();
         case iPolygon::Type::arc_segment:
            return ((cArcSegmentImpl*)m_pGeom)->rectangle();
         case iPolygon::Type::rectangle:
            return ((cRectImpl*)m_pGeom)->rectangle();
         case iPolygon::Type::polyline:
            return ((cShapeImpl*)m_pGeom)->rectangle();
      }
      return cRect();
   }

   bool equal(const iPolygon* ps, double tolerance = 0) const override
   {
      return false;
   }

   bool includes(const iPolygon* ps, bool open_set = false) const override
   {
      return false;
   }
   bool overlaps(const iPolygon* ps, bool open_set = false) const override
   {
      return false;
   }
   bool touches(const iPolygon* ps) const override
   {
      return false;
   }
   double length() const override
   {
      switch (type()) {
         case iPolygon::Type::circle:
            return ((cCircleImpl*)m_pGeom)->length();
         case iPolygon::Type::segment:
            return ((cSegmentImpl*)m_pGeom)->length();
         case iPolygon::Type::arc_segment:
            return ((cArcSegmentImpl*)m_pGeom)->length();
         case iPolygon::Type::polyline:
            return ((cShapeImpl*)m_pGeom)->length();
      }
      return 0;
   }
   double area() const override
   {
      switch (type()) {
         case iPolygon::Type::circle:
            return ((cCircleImpl*)m_pGeom)->area();
         case iPolygon::Type::segment:
            return ((cSegmentImpl*)m_pGeom)->area();
         case iPolygon::Type::arc_segment:
            return ((cArcSegmentImpl*)m_pGeom)->area();
         case iPolygon::Type::rectangle:
            return ((cRectImpl*)m_pGeom)->area();
         case iPolygon::Type::polyline:
            return ((cShapeImpl*)m_pGeom)->area();
      }
      return 0;
   }

   void add_vertex(double x, double y, coord_t bulge) override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            ((cShapeImpl*)m_pGeom)->add_vertex(x, y, bulge);
            break;
      }
   }
   bool add_arc(coord_t center_x, coord_t center_y, coord_t r, coord_t x, coord_t y, bool ccw = true) override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            return ((cShapeImpl*)m_pGeom)->add_arc(center_x, center_y, r, x, y, ccw);
      }
      return false;
   }
   bool add_hole(iPolygon* hole) override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            if (!m_pGeom->m_static) {
               m_holes->push_front((iGeomImpl*)hole);
               return true;
            }
            break;
      }
      return false;
   }
   void commit() override
   {
      switch (type()) {
         case iPolygon::Type::polyline:
            ((cShapeImpl*)m_pGeom)->commit();
      }
   }
   void release() override
   {
      delete this;
   }
   iShape* clone() override
   {
      auto retval = new cGeomImpl(m_pGeom);
      retval->m_holes = m_holes;
      return retval;
   }
};
