#pragma once

struct cLoaderPads : public cLoaderBase
{
   cLoaderVertex m_position;
   list<cPad *> m_pads;

   cLoaderPads(cXmlPcbSaxLoader* ldr)
      : cLoaderBase(ldr)
   {
   }
   void OnShapeAdded(cGeomImplBase *ps) override
   {
      m_pads.back()->setShape(ps);
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Position: {
            m_position = cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&m_position);
         } break;
         case eObject::Shape: {
            auto pad = m_ldr->m_db->createPad();
            m_pads.push_back(pad);
            auto shape = new cLoaderShape(m_ldr, atts, this, eObjId::MountingHole);
            pad->setLayer(shape->m_layer);
            m_ldr->m_loader_stack.push_back(shape);
         } break;
      }
   }
};
