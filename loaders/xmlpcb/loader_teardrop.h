#pragma once

#include "loader_base.h"

struct cLoaderTeardrop : public cLoaderBase
{
   cTeardrop *teardrop = nullptr;

   cLoaderTeardrop(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      teardrop = m_ldr->m_db->createTeardrop();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            teardrop->setName(value);
            break;
         case eKeyword::NetName:
            m_ldr->m_teardrops_map[value].push_back(teardrop);
            break;
         case eKeyword::Layer:
            teardrop->setLayer(atoi(value));
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Shape: {
            auto shape = new cLoaderShape(m_ldr, atts, this, eObjId::Teardrop);
            m_ldr->m_loader_stack.push_back(shape);
         } break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      add_shapes(teardrop);
      cLoaderBase::OnEndElement(name);
   }
};
