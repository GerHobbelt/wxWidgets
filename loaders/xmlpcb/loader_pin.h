#pragma once

#include "string_utils.h"

string_map<cPin *> m_pinmap;

struct cLoaderPin : public cLoaderBase
{
   cPin *pin = nullptr;
   cLoaderVertex position;
   const cChar *name = nullptr;
   const cChar *comp_name = nullptr;
   list<cPad *> m_pads;

   cLoaderPin(cXmlPcbSaxLoader *ldr, const cChar **atts, const cChar *parent_name, cPin *p = nullptr)
      : cLoaderBase(ldr)
      , comp_name(parent_name)
      , pin(p)
   {
      loadAttributes(atts, [this] ATT_HANDLER_SIG {
         switch (kw) {
            case eKeyword::Name:
               name = value;
               break;
            case eKeyword::Component:
               comp_name = value;
               break;
         }
      });

      const size_t bufsize = 1000;
      cChar buf[bufsize];
      strcpy_s<bufsize>(buf, comp_name);
      strcat_s<bufsize>(buf, ".");
      strcat_s<bufsize>(buf, name);

      if (pin) {
         pin->setName(buf);
         m_ldr->m_pinmap[buf] = pin;
      }
      else {
         auto it = m_ldr->m_pinmap.find(buf);
         if (it != m_ldr->m_pinmap.end()) {
            pin = it->second;
         }
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      if (auto it = s_object.find(name); it != s_object.end()) {
         switch (it->second) {
            case eObject::Position: {
               position = cLoaderVertex(m_ldr, atts);
               m_ldr->m_loader_stack.push_back(&position);
               pin->setPosition(position.m_point);
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
   }
   void OnEndElement(const cChar *name) override
   {
      include(pin, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderBase::OnEndElement(name);
   }
};
