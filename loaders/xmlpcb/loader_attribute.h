#pragma once

#include "string_utils.h"

string_map<cAttributeName *> m_attrmap;

struct cLoaderAttribute : public cLoaderBase
{
   cLoaderAttribute(cXmlPcbSaxLoader *ldr, const cChar **atts, cAttribute *attr)
      : cLoaderBase(ldr)
   {
      const cChar *name = nullptr, *val = nullptr;
      loadAttributes(atts, [this, &name, &val] ATT_HANDLER_SIG {
         switch (kw) {
            case eKeyword::Name:
               name = value;
               break;
            case eKeyword::Value:
               val = value;
               break;
         }
      });

      auto it = m_ldr->m_attrmap.find(name);
      if (it == m_ldr->m_attrmap.end()) {
         auto attrname = m_ldr->m_db->createAttributeName();
         attrname->setName(name);
         it = m_ldr->m_attrmap.emplace(attrname->getName(), attrname).first;
      }
      it->second->includeAttribute(*attr);
      attr->setValue(val);
   }
};
