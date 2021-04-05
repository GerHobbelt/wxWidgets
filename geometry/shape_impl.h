#pragma once

#include "geom_impl_base.h"
#include "geom_type_desc.h"

struct cSharedMemoryTraits : public cavc::PolylineTraits<coord_t>
{
   using Vertex = typename PVertex;
   using Allocator = shm::allocator<Vertex>;
   using Container = shm::vector<Vertex>;
};

using cPoly = cavc::Polyline<coord_t, cSharedMemoryTraits>;

struct cShapeImpl
   : public cGeomImplBase
   , public cPoly
{
   shm::offset_ptr<cGeomTypeDesc> m_holder;

   struct cVertexIterImpl : public iVertexIter
   {
      const cSharedMemoryTraits::Container& m_vertices;
      size_t m_idx = -1;

      cVertexIterImpl(const cSharedMemoryTraits::Container& vertices)
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

   cShapeImpl(iPolygon::Type type, bool hole, bool filled TAG)
      : cGeomImplBase(type, hole, filled PASS_TAG)
      , cPoly(shm::alloc<cSharedMemoryTraits::Vertex>())
   {
   }
   ~cShapeImpl()
   {
   }

   bool empty() const
   {
      return !cPoly::size();
   }
   bool closed() const
   {
      return cPoly::isClosed();
   }

   cVertexIter vertices() const
   {
      return new cVertexIterImpl(cPoly::vertexes());
   }

   struct cHolesIter : public iPolygonIter
   {
      const cGeomTypeDesc::shapes_t& m_shapes;
      size_t m_beg, m_end, m_idx;

      cHolesIter(const cGeomTypeDesc::shapes_t& shapes, size_t beg = 0, size_t end = -1)
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
      bool next(iPolygon** ps) override;
      size_t count() override
      {
         return m_end - m_beg;
      }

      iPolygonIter* clone() const override
      {
         return new cHolesIter(*this);
      }
   };

   void holes(iPolygonIter** res) const
   {
      *res = new cHolesIter(m_holder->m_shapes);
   }

   double length() const
   {
      return cavc::getPathLength(*this);
   }
   double area() const
   {
      return cavc::getArea(*this);
   }
   cRect rectangle() const
   {
      auto ext = cavc::getExtents(*this);
      return cRect(ext.xMin, ext.yMin, ext.xMax, ext.yMax);
   }

   void add_vertex(double x, double y, coord_t bulge)
   {
      cPoly::addVertex(x, y, bulge);
   }
   bool add_arc(coord_t center_x, coord_t center_y, coord_t r, coord_t x, coord_t y, bool ccw = true)
   {
      auto& last_vertex = cPoly::lastVertex();
      cPoint v1(last_vertex.x(), last_vertex.y()), v2(x, y);

      auto bulge = cArc(v1, v2, { center_x, center_y }, ccw ? r : -r, 0).m_bulge;
      last_vertex.bulge() = bulge;

      cPoly::addVertex(x, y, 0);

      return true;
   }
   void commit()
   {
      cPoly::isClosed() = true;
   }
};
