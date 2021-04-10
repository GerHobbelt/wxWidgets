#pragma once

struct cLoaderVia : public cLoaderBase
{
   cVia *via = nullptr;
   cLoaderVertex position;
   list<cPad *> m_pads;

   cLoaderVia(cXmlPcbSaxLoader *ldr, const cChar **atts, cVia *v)
      : cLoaderBase(ldr)
      , via(v)
   {
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::DrillSize:
            via->setDrillSize(fast_atod(value));
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Position: {
            position = cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&position);
            via->setPosition(position.m_point);
         } break;
         case eObject::Shape: {
            auto pad = m_ldr->m_db->createPad();
            m_pads.push_back(pad);
            auto shape = new cLoaderShape(m_ldr, atts, pad, eObjId::MountingHole);
            pad->setLayer(shape->m_layer);
            m_ldr->m_loader_stack.push_back(shape);
         } break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      include(via, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderBase::OnEndElement(name);
   }
};
