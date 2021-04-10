#pragma once

struct cLoaderUnroutedSegment : public cLoaderBase
{
   cLink *link = nullptr;
   cLoaderVertex beg, end;

   cLoaderUnroutedSegment(cXmlPcbSaxLoader *ldr, const cChar **atts, cLink *l)
      : cLoaderBase(ldr)
      , link(l)
   {
      loadAttributes(atts, [this] ATT_HANDLER_SIG {
         switch (kw) {
            case eKeyword::Layer1:
               link->setLayer1(atoi(value));
               break;
            case eKeyword::Layer2:
               link->setLayer2(atoi(value));
               break;
         }
      });
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      if (auto it = s_object.find(name); it != s_object.end()) {
         switch (it->second) {
            case eObject::Begin:
               beg = cLoaderVertex(m_ldr, atts);
               m_ldr->m_loader_stack.push_back(&beg);
               link->setBeg(beg.m_point);
               break;
            case eObject::End:
               end = cLoaderVertex(m_ldr, atts);
               m_ldr->m_loader_stack.push_back(&end);
               link->setEnd(end.m_point);
               break;
         }
      }
   }
};
