#pragma once

#include "loader_base.h"

struct cLoaderLayer : public cLoaderBase
{
   cLayer* m_layer = nullptr;
   int number = 0;

   cLoaderLayer(cXmlPcbSaxLoader* ldr, const cChar** atts)
      : cLoaderBase(ldr)
   {
      m_layer = m_ldr->m_db->createLayer();
      loadAttributes(atts);

      auto n_layer = m_layer->getLayerNumber();
      auto it = m_ldr->m_planes.find(n_layer);
      if (it == m_ldr->m_planes.end()) {
         it = m_ldr->m_planes.emplace(n_layer, m_ldr->m_db->create<cPlaneBase>(n_layer, m_layer->getName())).first; // 1-based layer numbering
      }

      m_layer->setPlane(it->second);

      assert(m_ldr->m_board);
      m_ldr->m_board->includeLayer(*m_layer);
   }
   void add_metal_layer()
   {
      m_ldr->m_el_layers.push_back(m_layer);
      auto metal_number = m_ldr->m_el_layers.size();
      m_ldr->m_metal_layers_map[metal_number] = m_layer;
      m_layer->setMetalLayerNumber(metal_number);
   }
   void attribute(eKeyword kw, const cChar* value) override
   {
      switch (kw) {
         case eKeyword::Name:
            m_layer->setName(value);
            break;
         case eKeyword::Number:
            number = atoi(value);
            m_layer->setLayerNumber(number + 1); // 1-based
            m_ldr->m_layers.push_back(m_layer);
            break;
         case eKeyword::Thickness:
            m_layer->setThickness(fast_atod(value));
            break;
         case eKeyword::Dielectric:
            m_layer->setDielectricConstant(fast_atod(value));
            break;
         case eKeyword::Type: {
            switch (int type = atoi(value)) {
               case 0:
                  add_metal_layer();
                  m_layer->setLayerType(eLayerType::Conductive);
                  m_layer->setConductiveLayerType(eConductiveLayerType::Signal);
                  break;
               case 1:
                  m_layer->setLayerType(eLayerType::Dielectric);
                  m_layer->setDielectricLayerType(eDielectricLayerType::Substrate);
                  break;
               case 2:
                  add_metal_layer();
                  m_layer->setLayerType(eLayerType::Conductive);
                  m_layer->setConductiveLayerType(eConductiveLayerType::Plane);
                  break;
               case 3:
                  add_metal_layer();
                  m_layer->setLayerType(eLayerType::Conductive);
                  m_layer->setConductiveLayerType(eConductiveLayerType::PlaneNegative);
                  break;
               case 4:
                  add_metal_layer();
                  m_layer->setLayerType(eLayerType::Conductive);
                  m_layer->setConductiveLayerType(eConductiveLayerType::Split);
                  break;
               case 5:
                  m_layer->setLayerType(eLayerType::Dielectric);
                  m_layer->setManufacturingLayerType(eManufacturingLayerType::SilkscreenTop);
                  break;
               case 6:
                  m_layer->setLayerType(eLayerType::Dielectric);
                  m_layer->setManufacturingLayerType(eManufacturingLayerType::SilkscreenBottom);
                  break;
               case 7:
                  m_layer->setLayerType(eLayerType::Dielectric);
                  m_layer->setDielectricLayerType(eDielectricLayerType::SolderMaskTop);
                  break;
               case 8:
                  m_layer->setLayerType(eLayerType::Dielectric);
                  m_layer->setDielectricLayerType(eDielectricLayerType::SolderMaskBottom);
                  break;
               case 9:
                  m_layer->setLayerType(eLayerType::Dielectric);
                  m_layer->setManufacturingLayerType(eManufacturingLayerType::SolderPasteTop);
                  break;
               case 10:
                  m_layer->setLayerType(eLayerType::Dielectric);
                  m_layer->setManufacturingLayerType(eManufacturingLayerType::SolderPasteBottom);
                  break;
               default:
                  m_layer->setLayerType(eLayerType::Unknown);
                  break;
            }
         } break;
      }
   }
};
