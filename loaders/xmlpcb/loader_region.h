#pragma once

#include "loader_bendarea.h"

struct cLoaderRegion : public cLoaderBase
{
   cBoardRegion *region = nullptr;
   eBoardRegionType::value type;
   const cChar *name;

   struct cLoaderRgnLayer : public cLoaderBase
   {
      int layer = 0;

      cLoaderRgnLayer(cXmlPcbSaxLoader* ldr, const cChar** atts)
         : cLoaderBase(ldr)
      {
         loadAttributes(atts);
      }
      void attribute(eKeyword kw, const cChar *value) override
      {
         switch (kw) {
            case eKeyword::Layer:
               layer = atoi(value);
               break;
         }
      }
   };
   struct cLoaderSubrgn : public cLoaderBase
   {
      const cChar *name = nullptr;

      cLoaderSubrgn(cXmlPcbSaxLoader *ldr, const cChar **atts)
         : cLoaderBase(ldr)
      {
         loadAttributes(atts);
      }
      void attribute(eKeyword kw, const cChar *value) override
      {
         switch (kw) {
            case eKeyword::Name:
               name = value;
               break;
         }
      }
   };

   cLoaderRegion(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      loadAttributes(atts);

      region = m_ldr->getRegion(name);
      region->setBoardRegionType(type);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            name = value;
            break;
         case eKeyword::Type:
            type = (eBoardRegionType::value)atoi(value);
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::BendArea: {
            auto bend_area_ldr = new cLoaderBendArea(m_ldr, atts);
            region->includeBendArea(*bend_area_ldr->barea);
            m_ldr->m_loader_stack.push_back(bend_area_ldr);
         } break;
         case eObject::Stackup: {
            auto rgn_layer_ldr = new cLoaderRgnLayer(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(rgn_layer_ldr);
            assert(m_ldr->m_layers.size() > rgn_layer_ldr->layer);
            region->includeLayer(*m_ldr->m_layers[rgn_layer_ldr->layer]);
         } break;
         case eObject::SubRegion: {
            auto subrgn_ldr = new cLoaderSubrgn(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(subrgn_ldr);
            region->includeBoardRegion(*m_ldr->getRegion(subrgn_ldr->name));
         } break;
         case eObject::Shape: {
            auto shape = new cLoaderShape(m_ldr, atts, region, eObjId::BoardRegion);
            m_ldr->m_loader_stack.push_back(shape);
         } break;
      }
   }
};
