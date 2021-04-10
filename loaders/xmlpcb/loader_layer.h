#pragma once

struct cLoaderLayer : public cLoaderBase
{
   cLoaderLayer(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      cLayer *layer = ldr->m_db->createLayer();

      loadAttributes(atts, [this, layer] ATT_HANDLER_SIG {
         switch (kw) {
            case eKeyword::Name:
               layer->setName(value);
               break;
            case eKeyword::Number:
               layer->setLayerNumber(atoi(value) + 1); // 1-based
               break;
            case eKeyword::Thickness:
               layer->setThickness(atof(value));
               break;
            case eKeyword::Dielectric:
               layer->setDielectricConstant(atof(value));
               break;
            case eKeyword::Type: {
               switch (int type = atoi(value)) {
                  case 0:
                     m_ldr->m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::Signal);
                     break;
                  case 1:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setDielectricLayerType(eDielectricLayerType::Substrate);
                     break;
                  case 2:
                     m_ldr->m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::Plane);
                     break;
                  case 3:
                     m_ldr->m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::PlaneNegative);
                     break;
                  case 4:
                     m_ldr->m_el_layers.push_back(layer);
                     layer->setLayerType(eLayerType::Conductive);
                     layer->setConductiveLayerType(eConductiveLayerType::Split);
                     break;
                  case 5:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SilkscreenTop);
                     break;
                  case 6:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SilkscreenBottom);
                     break;
                  case 7:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setDielectricLayerType(eDielectricLayerType::SolderMaskTop);
                     break;
                  case 8:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setDielectricLayerType(eDielectricLayerType::SolderMaskBottom);
                     break;
                  case 9:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SolderPasteTop);
                     break;
                  case 10:
                     layer->setLayerType(eLayerType::Dielectric);
                     layer->setManufacturingLayerType(eManufacturingLayerType::SolderPasteBottom);
                     break;
                  default:
                     layer->setLayerType(eLayerType::Unknown);
                     break;
               }
            } break;
         }
      });

      auto n_layer = layer->getLayerNumber();
      auto it = m_ldr->m_planes.find(n_layer);
      if (it == m_ldr->m_planes.end()) {
         m_ldr->m_planes.emplace(n_layer, m_ldr->m_ge->create_plane(n_layer, layer->getName())); // 1-based layer numbering
      }
   }
};
