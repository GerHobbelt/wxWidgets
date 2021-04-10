#pragma once

struct cLoaderNetClass : public cLoaderBase
{
   cNetClass *netclass = nullptr;
   list<cAttribute *> m_attribs;

   cLoaderNetClass(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      netclass = m_ldr->m_db->createNetClass();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            netclass->setName(value);
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Attribute: {
            cAttribute *attr = m_ldr->m_db->createAttribute();
            m_attribs.push_back(attr);
            m_ldr->m_loader_stack.push_back(new cLoaderAttribute(m_ldr, atts, attr));
         } break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      include(netclass, m_attribs, cDbTraits::eRelId::Comp_Attribute);
      cLoaderBase::OnEndElement(name);
   }
};
