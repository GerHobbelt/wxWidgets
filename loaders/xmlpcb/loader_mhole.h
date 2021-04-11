#pragma once

#include "loader_pads.h"

struct cLoaderMountingHole : public cLoaderPads
{
   cMountingHole *mhole = nullptr;

   cLoaderMountingHole(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderPads(ldr)
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
   void OnEndElement(const cChar *name) override
   {
      mhole->setPosition(m_position.m_point);
      include(mhole, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderBase::OnEndElement(name);
   }
};
