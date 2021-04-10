#pragma once

struct cLoaderMountingHole : public cLoaderBase
{
   cMountingHole *mhole = nullptr;
   cLoaderVertex position;
   list<cPad *> m_pads;

   cLoaderMountingHole(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      mhole = m_ldr->m_db->createMountingHole();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            mhole->setName(value);
            break;
         case eKeyword::NetName:
            m_ldr->m_mholes_map[value].push_back(mhole);
            break;
         case eKeyword::DrillSize:
            mhole->setDrillSize(atof(value));
            break;
         case eKeyword::Plated:
            mhole->setPlated(!!atoi(value));
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Position: {
            position = cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&position);
            mhole->setPosition(position.m_point);
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
      include(mhole, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderBase::OnEndElement(name);
   }
};
