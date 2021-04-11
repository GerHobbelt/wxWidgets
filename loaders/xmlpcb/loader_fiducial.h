#pragma once

#include "loader_pads.h"

struct cLoaderFiducial : public cLoaderPads
{
   cFiducial *fiducial = nullptr;
   string net_name;

   cLoaderFiducial(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderPads(ldr)
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
   void OnEndElement(const cChar *name) override
   {
      fiducial->setPosition(m_position.m_point);
      include(fiducial, m_pads, cDbTraits::eRelId::Object_Pad);
      cLoaderPads::OnEndElement(name);
   }
};
