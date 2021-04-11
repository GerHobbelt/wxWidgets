#pragma once

#include "loader_pads.h"
#include "string_utils.h"

string_map<cPin *> m_pinmap;

struct cLoaderPin : public cLoaderPads
{
   cPin *pin = nullptr;
   const cChar *name = nullptr;
   const cChar *comp_name = nullptr;

   cLoaderPin(cXmlPcbSaxLoader *ldr, const cChar **atts, const cChar *parent_name, cPin *p = nullptr)
      : cLoaderPads(ldr)
      , comp_name(parent_name)
      , pin(p)
   {
      loadAttributes(atts);

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
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            name = value;
            break;
         case eKeyword::Component:
            comp_name = value;
            break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      pin->setPosition(m_position.m_point);
      include(pin, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderBase::OnEndElement(name);
   }
};
