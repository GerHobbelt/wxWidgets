#pragma once

struct cLoaderSegment : public cLoaderBase
{
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
      auto v = new cLoaderVertex(m_ldr, atts);
      m_ldr->m_loader_stack.push_back(v);
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Begin:
            beg = v->m_point;
            break;
         case eObject::Center:
            is_arc = true;
            center = v->m_point;
            radius = v->m_radius;
            break;
         case eObject::End:
            end = v->m_point;
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
      cLayer* el_layer = m_ldr->m_el_layers[layer];
      el_layer->includeTrace(*trace);
      trace->setWidth(width);

      cLoaderBase::OnEndElement(name);
   }
};
