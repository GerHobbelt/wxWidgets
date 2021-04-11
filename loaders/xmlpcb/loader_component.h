#pragma once

struct cLoaderComponent : public cLoaderBase
{
   cComp *comp;
   list<cPin *> m_pins;
   list<cAttribute *> m_attribs;
   cLoaderVertex position;

   cLoaderComponent(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      comp = m_ldr->m_db->createComp();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            comp->setName(value);
            break;
         case eKeyword::Layer:
            comp->setLayer(atoi(value));
            break;
         case eKeyword::PartType:
            comp->setPartType(value);
            break;
         case eKeyword::Type: {
            int type = atoi(value);
            if (type < 7) {
               ++type;
            }
            else if (type == 7) {
               type = 0;
            }
            comp->setCompType((eCompType::value)type);
         } break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Position: {
            position = cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&position);
            comp->setPosition(position.m_point);
         } break;
         case eObject::Pin: {
            cPin *pin = m_ldr->m_db->createPin();
            m_pins.push_back(pin);
            m_ldr->m_loader_stack.push_back(new cLoaderPin(m_ldr, atts, comp->getName(), pin));
         } break;
         case eObject::Attribute: {
            cAttribute *attr = m_ldr->m_db->createAttribute();
            m_attribs.push_back(attr);
            m_ldr->m_loader_stack.push_back(new cLoaderAttribute(m_ldr, atts, attr));
         } break;
         case eObject::PlacementOutline: {
            m_ldr->skip();
            //cOutline *ol = m_ldr->m_db->createOutline();
            //comp->includePlacementOutline(*ol);
            //m_ldr->m_loader_stack.push_back(new cLoaderOutline(m_ldr, atts, ol, eObjId::Comp));
         } break;
         case eObject::Outline: {
            cOutline *ol = m_ldr->m_db->createOutline();
            comp->includeOutline(*ol);
            m_ldr->m_loader_stack.push_back(new cLoaderOutline(m_ldr, atts, ol, eObjId::Comp));
         } break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      include(comp, m_pins, cDbTraits::eRelId::Comp_Pin);
      include(comp, m_attribs, cDbTraits::eRelId::Comp_Attribute);

      cLoaderBase::OnEndElement(name);
   }
};
