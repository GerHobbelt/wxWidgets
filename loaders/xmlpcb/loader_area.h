#pragma once

struct cLoaderArea : public cLoaderBase
{
   cAreaFill* area = nullptr;
   int layer;

   cLoaderArea(cXmlPcbSaxLoader* ldr, const cChar** atts, cAreaFill* a)
      : cLoaderBase(ldr)
      , area(a)
   {
      loadAttributes(atts, [this] ATT_HANDLER_SIG{
         switch (kw) {
            case eKeyword::Layer:
               layer = atoi(value);
               break;
         }
         });
   }
   void OnStartElement(const cChar* name, const cChar** atts) override
   {
      m_ldr->m_loader_stack.push_back(new cLoaderShape(m_ldr, atts, area, eObjId::AreaFill, layer));
   }
};

