#pragma once

struct cLoaderBase
{
   cXmlPcbSaxLoader *m_ldr;
   cGeomImplBase * m_shape = nullptr;
   list<cGeomImplBase*> m_shapes;

   static constexpr double to_mils = 1.0 / 2540.0;

   cLoaderBase(cXmlPcbSaxLoader *ldr)
      : m_ldr(ldr)
   {
   }
   virtual ~cLoaderBase()
   {
   }

   void loadAttributes(const cChar **atts)
   {
      if (atts) {
         for (auto att = atts; *att; att += 2) {
            attribute((eKeyword)name2int(att[0]), att[1]);
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

   template <typename T>
   void add_shapes(T* obj)
   {
      auto n_shapes = m_shapes.size();
      if (!n_shapes) {
         if (m_shape) {
            obj->setShape(m_shape);
            m_shape->set_object(obj);
         }
         return;
      }

      auto group = m_ldr->m_db->create<cGeomImplGroup>();
      group->reserve(n_shapes + 1);
      group->set_object(obj);
      m_shape->set_object(obj);
      group->push_back(m_shape);
      for (auto &&shape: m_shapes) {
         shape->set_object(obj);
         group->push_back(shape);
      }
      obj->setShape(group);
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

   geom::coord_t get_coord(const cChar* value)
   {
      return fast_atod(value) * to_mils; // convert from 10s of nm to mils
   }

   virtual void Delete()
   {
      delete this;
   }

   virtual void attribute(eKeyword kw, const cChar* value)
   {
#define K(x) case eKeyword::x: break;
      switch (kw) {
#include "keywords.h"
      };
#undef K
   }

   virtual void OnStartElement(const cChar *name, const cChar **atts)
   {
#define K(x) case eObject::x: break;
      switch ((eObject)name2int(name)) {
#include "objects.h"
      };
#undef K
   }

   virtual void OnEndElement(const cChar *name)
   {
      assert(m_ldr->m_loader_stack.back() == this);
      m_ldr->m_loader_stack.pop_back();
      Delete();
   }

   virtual void OnShapeAdded(cGeomImplBase* ps)
   {
      if (!m_shape) {
         m_shape = ps;
      }
      else {
         m_shapes.push_back(ps);
      }
   }
};
