#pragma once

struct cLoaderBase
{
   cXmlPcbSaxLoader *m_ldr;

   cLoaderBase(cXmlPcbSaxLoader *ldr)
      : m_ldr(ldr)
   {
   }
   virtual ~cLoaderBase()
   {
   }

   void loadAttributes(const cChar **atts, function<void ATT_HANDLER_SIG> method)
   {
      for (auto att = atts; *att; att += 2) {
         auto it = s_keyword.find(att[0]);
         if (it != s_keyword.end()) {
            auto value = att[1];
            method(it->second, value);
         }
      }
   }

   template <typename P, typename L>
   void include(P &parent, L &objlist, cDbTraits::eRelId relid)
   {
      if (auto n = (int)objlist.size()) {
         auto rel = parent->get_relationship(relid, false, true);
         rel->resize(n);
         for (auto obj: objlist) {
            parent->include(relid, *obj);
         }
      }
   }

   void add_to_plane(cGeomImplBase *ps, int lay, eObjId type)
   {
      auto layer = lay;
      if (layer && layer <= m_ldr->m_el_layers.size()) {
         if (auto el_layer = m_ldr->m_el_layers[layer - 1]) {
            layer = el_layer->getLayerNumber();
         }
      }

      auto it = m_ldr->m_planes.find(layer);
      if (it != m_ldr->m_planes.end()) {
         cPlaneBase *plane = it->second;
         plane->add_shape(ps, (geom::ObjectType)type);
      }
   }

   virtual void Delete()
   {
      delete this;
   }

   virtual void OnStartElement(const cChar *name, const cChar **atts)
   {
   }
   virtual void OnEndElement(const cChar *name)
   {
      assert(m_ldr->m_loader_stack.back() == this);
      m_ldr->m_loader_stack.pop_back();
      Delete();
   }
};
