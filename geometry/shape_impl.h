#pragma once

#include "geom_impl.h"

using poly = cavc::Polyline<coord_t>;

struct cShapeImpl
   : public cGeomImpl
   , public poly
{
   Type m_type;

   struct cVertexIterImpl : public iVertexIter
   {
      const vector<cavc::PlineVertex<coord_t>>& m_vertices;
      size_t m_idx = -1;

      cVertexIterImpl(const vector<cavc::PlineVertex<coord_t>>& vertices)
         : m_vertices(vertices)
      {
      }
      cVertexIterImpl(const cVertexIterImpl& x)
         : m_vertices(x.m_vertices)
         , m_idx(x.m_idx)
      {
      }
      bool first(coord_t& vertex_x, coord_t& vertex_y) override
      {
         m_idx = -1;
         return next(vertex_x, vertex_y);
      }
      bool next(coord_t& vertex_x, coord_t& vertex_y) override
      {
         auto size = m_vertices.size();
         if (size && (m_idx == -1 || m_idx < size - 1)) {
            auto& v = m_vertices[++m_idx];
            vertex_x = v.x();
            vertex_y = v.y();
            return true;
         }
         return false;
      }
      size_t count() override
      {
         return m_vertices.size();
      }
      double bulge() override
      {
         auto size = m_vertices.size();
         if (size && m_idx < size - 1) {
            auto& v1 = m_vertices[m_idx];
            return v1.bulge();
         }
         return 0;
      }

      iVertexIter* clone() const override
      {
         return new cVertexIterImpl(*this);
      }
   };

   cShapeImpl(Type type, bool hole, bool filled TAG)
      : cGeomImpl(hole, filled PASS_TAG)
      , m_type(type)
   {
   }

   Type type() const override
   {
      return m_type;
   }

   bool empty() const override
   {
      return !poly::size();
   }
   bool closed() const override
   {
      return poly::isClosed();
   }

   cVertexIter vertices() const override
   {
      return new cVertexIterImpl(poly::vertexes());
   }

   struct cHolesIter : public iPolygonIter
   {
      const vector<ptr_type>& m_shapes;
      size_t m_beg, m_end, m_idx;

      cHolesIter(const vector<ptr_type>& shapes, size_t beg = 0, size_t end = -1)
         : m_shapes(shapes)
         , m_beg(beg)
         , m_end(end == -1 ? shapes.size() : end)
      {
      }
      cHolesIter(const cHolesIter& x)
         : m_shapes(x.m_shapes)
         , m_idx(x.m_idx)
         , m_beg(x.m_beg)
         , m_end(x.m_end)
      {
      }
      bool first(iPolygon** ps) override
      {
         m_idx = m_beg - 1;
         return next(ps);
      }
      bool next(iPolygon** ps) override
      {
         if (m_end && (m_idx == m_beg - 1 || m_idx < m_end - 1)) {
            *ps = m_shapes[++m_idx].get();
            return true;
         }
         *ps = nullptr;
         return false;
      }
      size_t count() override
      {
         return m_end - m_beg;
      }

      iPolygonIter* clone() const override
      {
         return new cHolesIter(*this);
      }
   };

   void holes(iPolygonIter** res) const override
   {
      *res = new cHolesIter(m_holder->m_shapes);
   }

   double length() const override
   {
      return cavc::getPathLength(*this);
   }
   double area() const override
   {
      return cavc::getArea(*this);
   }
   cRect rectangle() const override
   {
      auto ext = cavc::getExtents(*this);
      return cRect(ext.xMin, ext.yMin, ext.xMax, ext.yMax);
   }

   void add_vertex(double x, double y, coord_t bulge) override
   {
      poly::addVertex(x, y, bulge);
   }
   bool add_arc(coord_t center_x, coord_t center_y, coord_t r, coord_t x, coord_t y, bool ccw = true) override
   {
      auto& last_vertex = poly::lastVertex();
      cPoint v1(last_vertex.x(), last_vertex.y()), v2(x, y);

      auto bulge = cArc(v1, v2, { center_x, center_y }, ccw ? r : -r, 0).m_bulge;
      last_vertex.bulge() = bulge;

      poly::addVertex(x, y, 0);

      return true;
   }
   bool add_hole(iPolygon* hole) override
   {
      if (m_static) {
         return false;
      }
      auto p = (cHoleAttachment*)attachment(AttachmentType_Hole);
      if (!p) {
         add_attachment(make_unique<cHoleAttachment>());
         p = (cHoleAttachment*)attachment(AttachmentType_Hole);
      }
      assert(p);
      p->m_holes.emplace_back(hole);
      return true;
   }
   void commit() override
   {
      poly::isClosed() = true;
   }
};
