#pragma once

#include "loader_base.h"

string_map<list<cNet*>> m_netclassmap;

struct cLoaderNet : public cLoaderBase
{
   cNet *net = nullptr;
   list<cPin *> m_pins;
   list<cVia *> m_vias;
   list<cTrace *> m_traces;
   list<cAreaFill *> m_areas;
   list<cAttribute *> m_attrs;
   list<cLink *> m_links;

   cLoaderNet(cXmlPcbSaxLoader *ldr, const cChar **atts)
      : cLoaderBase(ldr)
   {
      net = m_ldr->m_db->createNet();
      loadAttributes(atts);
   }
   void attribute(eKeyword kw, const cChar *value) override
   {
      switch (kw) {
         case eKeyword::Name:
            net->setName(value);
            break;
         case eKeyword::NetClass: {
               auto it = m_ldr->m_netclassmap.find(value);
               if (it == m_ldr->m_netclassmap.end()) {
                  it = m_ldr->m_netclassmap.emplace(value, list<cNet*>()).first;
               }
               it->second.push_back(net);
            } break;
      }
   }
   void OnStartElement(const cChar *name, const cChar **atts) override
   {
      switch (auto obj_type = (eObject)name2int(name)) {
         case eObject::Pin: {
            cPin *pin = m_ldr->m_db->createPin();
            m_pins.push_back(pin);
            m_ldr->m_loader_stack.push_back(new cLoaderPin(m_ldr, atts, nullptr, pin));
         } break;
         case eObject::Via: {
            cVia *via = m_ldr->m_db->createVia();
            m_vias.push_back(via);
            m_ldr->m_loader_stack.push_back(new cLoaderVia(m_ldr, atts, via));
         } break;
         case eObject::Segment: {
            cTrace *trace = m_ldr->m_db->createTrace();
            m_traces.push_back(trace);
            m_ldr->m_loader_stack.push_back(new cLoaderSegment(m_ldr, atts, trace));
         } break;
         case eObject::Area: {
            cAreaFill *area = m_ldr->m_db->createAreaFill();
            m_areas.push_back(area);
            m_ldr->m_loader_stack.push_back(new cLoaderArea(m_ldr, atts, area));
         } break;
         case eObject::Attribute: {
            cAttribute *attr = m_ldr->m_db->createAttribute();
            m_attrs.push_back(attr);
            m_ldr->m_loader_stack.push_back(new cLoaderAttribute(m_ldr, atts, attr));
         } break;
         case eObject::UnroutedSegment: {
            cLink *link = m_ldr->m_db->createLink();
            m_links.push_back(link);
            m_ldr->m_loader_stack.push_back(new cLoaderUnroutedSegment(m_ldr, atts, link));
         } break;
         default:
            assert(false);
            break;
      }
   }
   void OnEndElement(const cChar *name) override
   {
      include(net, m_pins, cDbTraits::eRelId::Net_Pin);
      include(net, m_vias, cDbTraits::eRelId::Net_Via);
      include(net, m_traces, cDbTraits::eRelId::Net_Trace);
      include(net, m_areas, cDbTraits::eRelId::Net_AreaFill);
      include(net, m_attrs, cDbTraits::eRelId::Net_Attribute);
      include(net, m_links, cDbTraits::eRelId::Net_Link);

      auto it = m_ldr->m_mholes_map.find(net->getName());
      if (it != m_ldr->m_mholes_map.end()) {
         include(net, it->second, cDbTraits::eRelId::Net_MountingHole);
      }

      cLoaderBase::OnEndElement(name);
   }
};
