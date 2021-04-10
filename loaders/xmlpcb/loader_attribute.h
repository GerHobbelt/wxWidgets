#pragma once

#include "string_utils.h"

string_map<cAttributeName *> m_attrnamemap;
string_map<list<cAttribute *>> m_attrmap;

struct cLoaderAttribute : public cLoaderBase
{
   const cChar *m_name = nullptr, *m_val = nullptr;

   cLoaderAttribute(cXmlPcbSaxLoader* ldr, const cChar** atts, cAttribute* attr)
      : cLoaderBase(ldr)
   {
      loadAttributes(atts);

      m_ldr->m_attrmap[m_name].push_back(attr);
      attr->setValue(m_val);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            m_name = value;
            break;
         case eKeyword::Value:
            m_val = value;
            break;
      }
   }
};
