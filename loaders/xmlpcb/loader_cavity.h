#pragma once

struct cLoaderCavity : public cLoaderBase
{
   cCavity *cavity = nullptr;

   cLoaderCavity(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      cavity = m_ldr->m_db->createCavity();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            cavity->setName(value);
            break;
         case eKeyword::Layer1:
            cavity->setStartLayer(atoi(value));
            break;
         case eKeyword::Layer2:
            cavity->setEndLayer(atoi(value));
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Shape: {
            auto shape = new cLoaderShape(m_ldr, atts, this, eObjId::Cavity);
            m_ldr->m_loader_stack.push_back(shape);
         } break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      add_shapes(cavity);
      cLoaderBase::OnEndElement(name);
   }
};
