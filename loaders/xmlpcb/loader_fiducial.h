#pragma once

struct cLoaderFiducial : public cLoaderBase
{
   cFiducial *fiducial = nullptr;
   cLoaderVertex position;
   list<cPad *> m_pads;
   string net_name;

   cLoaderFiducial(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      fiducial = m_ldr->m_db->createFiducial();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            fiducial->setName(value);
            break;
         case eKeyword::NetName:
            m_ldr->m_fiducials_map[value].push_back(fiducial);
            break;
         case eKeyword::Side:
            fiducial->setSide(atoi(value));
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Position: {
            position = cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&position);
            fiducial->setPosition(position.m_point);
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
      include(fiducial, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderBase::OnEndElement(name);
   }
};
