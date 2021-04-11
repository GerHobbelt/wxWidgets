#pragma once

#include <array>

#include "geom_model.h"
#include "../geometry/geom_engine_base.h"

#include "db_vector.h"
#include "db_introspector.h"
#include "db_object.h"
#include "db_relationship.h"

enum class DistanceUnit { mil, mm };

extern geom::iEngine *GetGeomEngine();
extern cGeomEngineBase *GetGeomEngineBase();

namespace db {

using namespace std;

template <typename Traits>
class cDatabase
{
   using types = cTypes<Traits>;
   using cObject = typename types::cObject;
   using cObjectPtr = typename types::cObjectPtr;
   using cObjDesc = typename types::cObjDesc;
   using cObjDescPtr = typename types::cObjDescPtr;
   using eObjId = typename types::eObjId;
   using uid_t = typename types::uid_t;

   using cPagePtr = typename types::template alloc_traits<cPageBase<Traits>>::pointer;

   struct cTypeDesc
   {
      cPagePtr m_first, m_last;

      cObjectPtr m_free;
      size_t m_objdesc_idx = -1;
      uid_t m_uid = 0;

      static inline auto &introspector = Traits::introspector;

      ~cTypeDesc()
      {
         auto &desc = introspector.m_obj_desc[m_objdesc_idx];
         while (m_first) {
            auto pPage = exchange(m_first, m_first->m_next);
            desc.m_page_disposer(pPage);
         }
      }

      cObjectPtr create(eObjId id)
      {
         auto &desc = introspector.m_obj_desc[m_objdesc_idx];
         cObjectPtr retval = nullptr;
         if (!m_free) {
            assert(desc.m_page_factory);
            auto [pNewPage, free_list] = desc.m_page_factory();
            m_free = free_list;

            pNewPage->m_next = nullptr;
            pNewPage->m_prev = m_last;
            if (m_last) {
               m_last->m_next = pNewPage;
            }
            else {
               assert(!m_first);
               m_first = pNewPage;
            }
            m_last = pNewPage;
         }
         retval = exchange(m_free, m_free->m_next_free);
         assert(desc.m_construct);
         desc.m_construct(&*retval, ++m_uid);
         return retval;
      }
      void erase(cObjectPtr pObj)
      {
         if (pObj->is_valid()) {
            pObj->remove_all_relationships();
            auto &desc = introspector.m_obj_desc[m_objdesc_idx];
            assert(desc.m_destruct);
            desc.m_destruct(&*pObj);
            pObj->m_next_free = m_free;
            m_free = pObj;
         }
      }
   };

public:
   template <Object<Traits> T>
   class const_iterator
   {
      using types = cTypes<Traits>;
      using cObject = typename types::cObject;
      using cObjectPtr = typename types::cObjectPtr;

      cPage<Traits, T> *m_page;
      uint16_t m_idx;

   public:
      const_iterator() = default;
      const_iterator(const const_iterator& x) = default;
      bool operator==(const const_iterator&x) const
      {
         return m_page == x.m_page && m_idx == x.m_idx;
      }
      auto& operator++()
      {
         do {
            if (++m_idx >= page_size) {
               m_idx = 0;
               m_page = (cPage<Traits, T> *)m_page->m_next;
            }
         } while (!m_page->m_objects[m_idx].is_valid());
         return *this;
      }
      const T& operator*() const
      {
         return m_page->m_objects[m_idx];
      }
      const T* operator->() const
      {
         return &this->operator*();
      }
   };

   template <Object<Traits> T>
   class const_iterator_range
   {
      const cDatabase* m_parent;
      const eObjId m_obj_id;

   public:
      const_iterator_range(const cDatabase* parent, eObjId obj_id)
         : m_parent(parent)
         , m_obj_id(obj_id)
      {
      }

      auto begin() const
      {
         cTypeDesc& desc = m_parent->m_objects[size_t(m_obj_id)];
         auto beg = desc.m_objects.cbegin();
         return const_iterator<T>(beg);
      }
      auto end() const
      {
         cTypeDesc& desc = m_parent->m_objects[size_t(m_obj_id)];
         auto end = desc.m_objects.cend();
         return const_iterator<T>(end);
      }
   };

   template <Object<Traits> T>
   class iterator
   {
      using types = cTypes<Traits>;
      using cObject = typename types::cObject;
      using cObjectPtr = typename types::cObjectPtr;

      cPage<Traits, T> *m_page;
      uint16_t m_idx;

   public:
      iterator() = default;
      iterator(const iterator& x) = default;
      iterator(cPagePtr page, uint16_t idx = 0)
         : m_page((cPage<Traits, T>*)&*page)
         , m_idx(idx)
      {
      }
      bool operator==(const iterator &x) const
      {
         return m_page == x.m_page && m_idx == x.m_idx;
      }
      auto& operator++()
      {
         do {
            if (++m_idx >= page_size) {
               m_idx = 0;
               m_page = (cPage<Traits, T> *)&*m_page->m_next;
            }
         } while (m_page && !m_page->m_objects[m_idx].is_valid());
         return *this;
      }
      const T& operator*() const
      {
         return m_page->m_objects[m_idx];
      }
      const T* operator->() const
      {
         return &this->operator*();
      }
   };

   template <Object<Traits> T>
   class iterator_range
   {
      cDatabase* m_parent;
      const eObjId m_obj_id;

   public:
      iterator_range(cDatabase* parent, eObjId obj_id)
         : m_parent(parent)
         , m_obj_id(obj_id)
      {
      }

      auto begin() const
      {
         cTypeDesc& desc = m_parent->m_objects[size_t(m_obj_id)];
         return iterator<T>(&*desc.m_first);
      }
      auto end() const
      {
         return iterator<T>();
      }
   };

   cObjectPtr create(eObjId id)
   {
      auto& obj_list = m_objects[size_t(id)];
      return obj_list.create(id);
   }
   const char* object_type_name(eObjId id)
   {
      if (auto idx = size_t(id); idx < m_objects.size()) {
         auto &obj_list = m_objects[idx];
         auto &desc = Traits::introspector.m_obj_desc;
         return desc[obj_list.m_objdesc_idx].m_name.c_str();
      }
      return nullptr;
   }
   void erase(cObjectPtr pObj)
   {
      if (pObj) {
         eObjId id = pObj->type();
         auto& type_list = m_objects[int(id)];
         type_list.erase(pObj);
      }
   }

public:
   geom::iEngine *geom_engine()
   {
      return GetGeomEngine();
   }
   cGeomEngineBase *geom_engine_base()
   {
      return GetGeomEngineBase();
   }

   void set_distance_units(DistanceUnit unit)
   {
      m_unit = unit;
   }
   void set_board_extents(double x1, double y1, double x2, double y2)
   {
      m_x1 = x1;
      m_x2 = x2;
      m_y1 = y1;
      m_y2 = y2;
   }
   void set_layer_number(size_t nLayers)
   {
      m_nLayers = nLayers;
   }

   template <typename T, typename... Args>
   static auto create(Args &&...args)
   {
      typename Traits::template alloc<T> a;
      auto new_p = a.allocate(1);

      Traits::template alloc_traits<T>::template construct(a, new_p, std::forward<Args>(args)...);
      return &*new_p;
   }

public:
   DistanceUnit m_unit = DistanceUnit::mil;
   double m_x1 = 0, m_y1 = 0, m_x2 = 0, m_y2 = 0;
   size_t m_nLayers = 0;

   cDatabase()
   {
      for (size_t id = 0; id < size(m_objects); ++id) {
         m_objects[id].m_objdesc_idx = Traits::introspector.find_obj_desc((eObjId)id);
      }
   }
   ~cDatabase()
   {
      Traits::s_objcount = 0;
   }

protected:
   array<cTypeDesc, size_t(eObjId::_count)> m_objects;
};

} // namespace db
