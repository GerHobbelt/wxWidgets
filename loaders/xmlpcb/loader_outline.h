#pragma once

struct cLoaderOutline : public cLoaderBase
{
   cOutline *outline = nullptr;
   eObjId type;

   cLoaderOutline(cXmlPcbSaxLoader *ldr, const cChar **atts, cOutline *ol, eObjId geom_type)
      : cLoaderBase(ldr)
      , outline(ol)
      , type(geom_type)
   {
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      m_ldr->m_loader_stack.push_back(new cLoaderShape(m_ldr, atts, outline, type));
   }
};
