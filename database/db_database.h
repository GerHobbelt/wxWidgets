#pragma once

#include <array>

namespace db {

#include "db_object.h"
#include "db_relationship.h"

using namespace std;

template <typename Traits>
class cDatabase : public cTypes<Traits>
{
   using types = cTypes<Traits>;
   using cObject = typename types::cObject;
   using cObjectPtr = typename types::cObjectPtr;
   using cObjDesc = typename types::cObjDesc;
   using eObjId = typename types::eObjId;
   using uid_t = typename types::uid_t;

   using cObjList = bin::list<cObject>;

   struct cTypeDesc
   {
      cObjList m_objects, m_spare;
      cObjDesc* m_objdesc = nullptr;
      uid_t m_uid = 0;

      ~cTypeDesc()
      {
         if (m_objdesc) {
            auto cleanup_list = [this](auto& list) {
               while (list.size()) {
                  auto pObj = &list.front();
                  list.erase(list.s_iterator_to(*pObj));
                  m_objdesc->m_disposer(pObj);
               }
            };
            cleanup_list(m_objects);
            cleanup_list(m_spare);
         }
      }

      cObjectPtr create(eObjId id)
      {
         cObjectPtr retval = nullptr;
         if (!m_spare.empty()) {
            retval = &m_spare.front();
            m_spare.pop_front();
         }
         else {
            if (!m_objdesc) {
               m_objdesc = Traits::introspector.find_obj_desc(id);
            }
            assert(m_objdesc->m_factory);
            retval = m_objdesc->m_factory();
         }
         retval->set_uid(++m_uid);
         m_objects.push_back(*retval);
         return retval;
      }
      void erase(cObjectPtr pObj)
      {
         if (pObj->uid()) {
            m_objects.erase(m_objects.s_iterator_to(*pObj));
            pObj->set_uid(0);
            m_spare.push_back(*pObj);
         }
      }
   };

public:
   template <Object<Traits> T>
   class const_iterator : public cTypes<Traits>
   {
      using types = cTypes<Traits>;
      using cObject = typename types::cObject;
      using cObjectPtr = typename types::cObjectPtr;

      typename cObjList::const_iterator m_iter;

   public:
      const_iterator() = default;
      const_iterator(const const_iterator& x) = default;
      const_iterator(const typename cObjList::const_iterator& x)
         : m_iter(x)
      {
      }
      bool operator==(const const_iterator& x) const
      {
         return m_iter == x.m_iter;
      }
      auto& operator++()
      {
         ++m_iter;
         return *this;
      }
      const T* operator->() const
      {
         return (T*)this->m_iter.operator->();
      }
      const T& operator*() const
      {
         return *operator->();
      }
   };

   template <Object<Traits> T>
   class const_iterator_range : public cTypes<Traits>
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
   class iterator : public cTypes<Traits>
   {
      using types = cTypes<Traits>;
      using cObject = typename types::cObject;
      using cObjectPtr = typename types::cObjectPtr;

      typename cObjList::iterator m_iter;

   public:
      iterator() = default;
      iterator(const iterator& x) = default;
      iterator(const typename cObjList::iterator& x)
         : m_iter(x)
      {
      }
      bool operator==(const iterator& x) const
      {
         return m_iter == x.m_iter;
      }
      auto& operator++()
      {
         ++m_iter;
         return *this;
      }
      const T* operator->() const
      {
         return (T*)this->m_iter.operator->();
      }
      const T& operator*() const
      {
         return *operator->();
      }
   };

   template <Object<Traits> T>
   class iterator_range : public cTypes<Traits>
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
         auto beg = desc.m_objects.begin();
         return iterator<T>(beg);
      }
      auto end() const
      {
         cTypeDesc& desc = m_parent->m_objects[size_t(m_obj_id)];
         auto end = desc.m_objects.end();
         return iterator<T>(end);
      }
   };

   cObjectPtr create(eObjId id)
   {
      auto& obj_list = m_objects[size_t(id)];
      return obj_list.create(id);
   }
   void erase(cObjectPtr pObj)
   {
      if (pObj) {
         pObj->remove_all_relationships();

         eObjId id = pObj->type();
         auto& type_list = m_objects[int(id)];
         type_list.erase(pObj);
      }
   }

protected:
   array<cTypeDesc, size_t(eObjId::_count)> m_objects;
};

} // namespace db
