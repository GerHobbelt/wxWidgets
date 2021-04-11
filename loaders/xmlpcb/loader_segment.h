#pragma once

#include "loader_base.h"

struct cLoaderSegment : public cLoaderBase
{
   cLoaderVertex m_vertex;
   cTrace *trace = nullptr;
   bool tie_leg = false, is_arc = false;
   geom::cPoint beg, center, end;
   geom::coord_t width, radius;
   int layer = 0;

   cLoaderSegment(cXmlPcbSaxLoader *ldr, const cChar **atts, cTrace *t)
      : cLoaderBase(ldr)
      , trace(t)
   {
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Width:
            width = get_coord(value);
            break;
         case eKeyword::Layer:
            layer = atoi(value);
            break;
         case eKeyword::TieLeg:
            tie_leg = !!atoi(value);
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      m_vertex = cLoaderVertex(m_ldr, atts);
      m_ldr->m_loader_stack.push_back(&m_vertex);
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Begin:
            beg = m_vertex.m_point;
            break;
         case eObject::Center:
            is_arc = true;
            center = m_vertex.m_point;
            radius = m_vertex.m_radius;
            break;
         case eObject::End:
            end = m_vertex.m_point;
            break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      cGeomImplBase* ps;
      if (is_arc) {
         ps = m_ldr->m_db->create<cArcSegmentImpl>(false, false, beg, end, center, radius, width PASS_TAG);
      }
      else {
         ps = m_ldr->m_db->create<cSegmentImpl>(false, false, beg, end, width PASS_TAG);
      }
      add_to_plane(ps, layer, eObjId::Trace);

      trace->setBeg(beg);
      trace->setEnd(end);
      m_ldr->m_traces_map[layer - 1].push_back(trace);
      trace->setWidth(width);
      trace->setShape(ps);
      ps->set_object(trace);

      cLoaderBase::OnEndElement(name);
   }
};
