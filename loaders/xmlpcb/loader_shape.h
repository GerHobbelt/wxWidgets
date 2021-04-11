#pragma once

cShapeImpl *m_current_shape = nullptr;

struct cLoaderShape : public cLoaderBase
{
   eObjId m_type;
   int m_layer = 0;
   cLoaderVertex m_center;
   geom::cPoint m_size;
   cGeomImplBase *m_ps = nullptr;
   geom::coord_t m_diameter = 0, m_width = 0;
   bool m_hole = false, m_filled = false, m_closed = false;
   eShapeType m_shape_type = eShapeType::Unknown;
   list<cLoaderVertex> m_vertices;
   cLoaderBase* m_parent_ldr;

   cLoaderShape(cXmlPcbSaxLoader *ldr, const cChar **atts, cLoaderBase* parent_ldr, eObjId type, int l = 0)
      : cLoaderBase(ldr)
      , m_parent_ldr(parent_ldr)
      , m_type(type)
      , m_layer(l)
   {
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Type:
            m_shape_type = (eShapeType)atoi(value);
            break;
         case eKeyword::Void:
            m_hole = !!atoi(value);
            break;
         case eKeyword::Filled:
            m_filled = !!atoi(value);
            break;
         case eKeyword::Closed:
            m_closed = !!atoi(value);
            break;
         case eKeyword::Layer:
            if (int layer = atoi(value); layer >= 0) {
               m_layer = layer;
            }
            break;
         case eKeyword::Diameter:
            m_diameter = get_coord(value);
            break;
         case eKeyword::Width:
            m_width = get_coord(value);
            break;
         case eKeyword::SizeX:
            m_size.m_x = get_coord(value);
            break;
         case eKeyword::SizeY:
            m_size.m_y = get_coord(value);
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Center:
            m_center = cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&m_center);
            break;
         case eObject::Vertex:
            m_vertices.emplace_back(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&m_vertices.back());
            break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      switch (m_shape_type) {
         case eShapeType::Round: {
            auto pt = m_center.m_point;
            m_ps = m_ldr->m_db->create<cCircleImpl>(m_hole, m_filled, pt.m_x, pt.m_y, m_diameter / 2 PASS_TAG);
         } break;
         case eShapeType::Square:
         case eShapeType::Rectangle: {
            auto pt = m_center.m_point;
            auto lb = pt - m_size / 2, rt = pt + m_size / 2;
            m_ps = m_ldr->m_db->create<cRectImpl>(m_hole, m_filled, lb.m_x, lb.m_y, rt.m_x, rt.m_y PASS_TAG);
         } break;
         case eShapeType::Oval:
            assert(false); //TBD
            break;
         case eShapeType::Finger:
            assert(false); //TBD
            break;
         case eShapeType::Path:
         case eShapeType::Polygon: {
            if (auto size = m_vertices.size()) {
               auto ps = m_ldr->m_db->create<cShapeImpl>(iPolygon::Type::polyline, m_hole, m_filled PASS_TAG);
               ps->reserve(size);
               bool prev_arc = false;
               geom::coord_t radius;
               geom::cPoint arc_center;
               for (auto &v: m_vertices) {
                  if (v.m_arc) {
                     arc_center = v.m_point;
                     prev_arc = v.m_arc;
                     radius = v.m_radius;
                  }
                  else if (prev_arc) {
                     ps->add_arc(arc_center.m_x, arc_center.m_y, radius, v.m_point.m_x, v.m_point.m_y, true);
                     prev_arc = false;
                  }
                  else {
                     ps->add_vertex(v.m_point.m_x, v.m_point.m_y, 0);
                  }
               }
               ps->commit();
               if (!m_hole) {
                  m_ldr->m_current_shape = ps;
               }
               m_ps = ps;
            }
         } break;
      }

      add_to_plane(m_ps, m_layer, m_type);

      if (m_hole) {
         assert(m_ldr->m_current_shape);
         m_ldr->m_current_shape->add_hole(m_ps);
      }

      m_parent_ldr->OnShapeAdded(m_ps);

      cLoaderBase::OnEndElement(name);
   }
};
