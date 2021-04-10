#pragma once

struct cLoaderBendArea : public cLoaderBase
{
   cBendArea *barea = nullptr;

   cLoaderBendArea(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      barea = m_ldr->m_db->createBendArea();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      //switch (kw) {
      //}
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Shape: {
            auto shape = new cLoaderShape(m_ldr, atts, barea, eObjId::BendArea);
            m_ldr->m_loader_stack.push_back(shape);
         } break;
      }
   }
};
