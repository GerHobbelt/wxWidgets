#pragma once

struct cLoaderSegment : public cLoaderBase
{
   cTrace *trace = nullptr;
   geom::cPoint beg, end;
   geom::coord_t width;
   int layer;

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
            width = fast_atod(value);
            break;
         case eKeyword::Layer:
            layer = atoi(value);
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
         case eObject::End:
            end = v->m_point;
            break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      cGeomImplBase *ps = shm::construct<cSegmentImpl>(false, false, beg, end, width PASS_TAG);
      add_to_plane(ps, layer, eObjId::Trace);

      trace->setBeg(beg);
      trace->setEnd(end);
      trace->setLayer(layer);
      trace->setWidth(width);

      cLoaderBase::OnEndElement(name);
   }
};
