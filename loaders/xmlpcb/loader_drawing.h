#pragma once

struct cLoaderDrawing : public cLoaderBase
{
   cDrawing *drawing = nullptr;
   int layer = 0;

   cLoaderDrawing(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      drawing = m_ldr->m_db->createDrawing();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Layer:
            for (auto l : m_ldr->m_layers) {
               if (!strcmp(value, l->getName())) {
                  layer = l->getLayerNumber();
                  drawing->setLayer(layer);
               }
            }
            break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Shape: {
            auto shape = new cLoaderShape(m_ldr, atts, this, eObjId::Drawing, layer);
            m_ldr->m_loader_stack.push_back(shape);
         } break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      add_shapes(drawing);
      cLoaderBase::OnEndElement(name);
   }
};
