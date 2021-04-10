#pragma once

struct cLoaderText : public cLoaderBase
{
   cText *text = nullptr;
   cLoaderVertex position;
   int layer;

   cLoaderText(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      text = m_ldr->m_db->createText();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Text:
            text->setText(value);
            break;
         case eKeyword::Layer:
            layer = atoi(value);
            if (auto it = m_ldr->m_metal_layers_map.find(layer); it != m_ldr->m_metal_layers_map.end()) {
               text->setLayer(it->second->getLayerNumber());
            }
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Position: {
            position = cLoaderVertex(m_ldr, atts);
            m_ldr->m_loader_stack.push_back(&position);
            text->setPosition(position.m_point);
         } break;
      }
   }
};
