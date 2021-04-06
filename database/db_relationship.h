#pragma once

#include "boost/interprocess/offset_ptr.hpp"

#include "db_vector.h"
#include "db_object.h"

namespace db {

using namespace std;
namespace bi = boost::interprocess;

template <typename Traits, typename Alloc = typename Traits::template alloc<cObjectPtr<Traits>>>
struct cRelationshipTraits
{
   using obj_ptr = cObjectPtr<Traits>;

   union value_type
   {
      value_type()
         : m_child(nullptr)
      {
      }
      value_type(const value_type& x)
         : m_child(x.m_child)
      {
      }
      value_type(const obj_ptr& child)
         : m_child(child)
      {
      }
      using free_ptr = bi::offset_ptr<value_type>;
      obj_ptr m_child;
      free_ptr m_next_free;
   };

   using free_pointer = typename value_type::free_ptr;

   using alloc = typename allocator_traits<Alloc>::template rebind_alloc<value_type>;
   using pointer = typename allocator_traits<alloc>::pointer;

   using cObject = cObject<Traits>;

   struct cData
   {
      int16_t m_idx;
      alloc m_alloc;
      bool m_parent_ref = false;
      union
      {
         int m_size;
         int m_parent_idx;
      };
      union
      {
         pointer m_data;
         obj_ptr m_object;
      };

      cData(size_t idx)
         : m_idx(int(idx))
         , m_data(nullptr)
         , m_size(0)
      {
      }
      cData(cData&& x)
         : m_idx(exchange(x.m_idx, 0))
         , m_size(exchange(x.m_size, 0))
         , m_parent_ref(exchange(x.m_parent_ref, false))
         , m_data(exchange(x.m_data, nullptr))
      {
      }
   };

   static void construct(pointer pos, alloc& a)
   {
      pos->m_child = nullptr;
   }
   template <typename T>
   static void construct(pointer pos, alloc& a, T arg)
   {
      pos->m_child = arg;
   }
   static void move_n(pointer dest, pointer src, size_t size, alloc& a)
   {
      memcpy(dest, src, size * sizeof(value_type));
   }
   static void destroy(pointer pos, alloc& a)
   {
   }
   static void destroy_n(pointer pos, size_t size, alloc& a)
   {
      memset(pos, 0, size * sizeof(value_type));
   }

   using vector = vector<value_type, cRelationshipTraits>;
};

template <typename Traits>
class cRelationship : public cRelationshipTraits<Traits>::vector
{
   template <typename Traits>
   friend class cObject;

protected:
   template <typename T>
   using alloc = typename Traits::template alloc<T>;

   template <typename T>
   using alloc_traits = allocator_traits<alloc<T>>;

   using cIntrospector = decltype(Traits::introspector);

   using cRelDescs = typename decltype(cIntrospector::m_rel_desc);
   using cRelDesc = typename cRelDescs::value_type;

   using eRelationshipType = typename cIntrospector::eRelationshipType;

public:
   using cRelationshipTraits = cRelationshipTraits<Traits>;

protected:
   using cObject = typename cRelationshipTraits::cObject;
   using cObjectPtr = typename cRelationshipTraits::obj_ptr;
   using cValuePtr = typename cRelationshipTraits::pointer;
   using cFreePtr = typename cRelationshipTraits::free_pointer;

   using base = typename cRelationshipTraits::vector;
   using value_type = typename base::value_type;

public:
   using base::m_idx, base::m_parent_idx, base::m_object;
   using base::m_parent_ref, base::m_data, base::m_size;

   cRelationship(size_t idx = -1) noexcept
      : base(cRelationshipTraits::cData(idx))
   {
   }
   cRelationship(cRelationship&& x) noexcept
      : base(move(x))
   {
   }
   ~cRelationship() noexcept
   {
      if (m_parent_ref) {
         m_parent_idx = 0;
         m_object = nullptr;
         m_parent_ref = 0;
      }
      else {
         if (0/*TBD: strong rel*/) {
            for (auto i = 0; i < m_size; ++i) {
               //base::destroy(m_data + i);
            }
         }
      }
      m_idx = -1;
   }

#ifndef TESTING
protected:
#endif
   bool is_valid() const noexcept
   {
      return m_idx >= 0;
   }
   size_t count() const noexcept
   {
      if (m_parent_ref) {
         return m_parent_idx >= 0 ? 1 : 0;
      }
      auto retval = m_size;
      for (auto free = first_free(); free; free = next_free(free)) {
         --retval;
      }
      return retval;
   }
   void resize(int new_size)
   {
      auto old_size = m_size;
      auto old_data = m_data;
      auto old_free_list = free_list_header();

      auto delta = new_size - old_size;
      base::resize(new_size);

      if (old_free_list) {
         auto old_idx = old_free_list - old_data;
         base::last()->m_next_free = m_data + old_idx;
      }
      m_data[m_size - 1].m_child = nullptr;
      for (auto i = delta - 1; i > 0;) {
         push_free(m_data + --i);
      }
   }

   cRelDesc& desc() const
   {
      return Traits::introspector.m_rel_desc[m_idx];
   }

   auto parent() const
   {
      return tuple(m_parent_ref ? m_object : nullptr, m_parent_idx);
   }

   bool is_free(const value_type& x) const
   {
      auto p = x.m_next_free.get();
      return p && size_t(p - m_data) < m_size;
   }
   cFreePtr free_list_header() const noexcept
   {
      auto free_list = base::last();
      if (free_list && is_free(*free_list) && is_free(*free_list->m_next_free)) {
         return cFreePtr(free_list);
      }
      return nullptr;
   }
   cFreePtr first_free() const noexcept
   {
      if (auto header = free_list_header()) {
         if (auto next = header->m_next_free) {
            return next;
         }
         return header;
      }
      return nullptr;
   }
   cFreePtr next_free(const cFreePtr& cur) const noexcept
   {
      if (cur != free_list_header()) {
         if (auto next = cur->m_next_free) {
            return next;
         }
      }
      return nullptr;
   }
   void push_free(cValuePtr x) noexcept
   {
      assert(x >= m_data && x < m_data + m_size);

      auto free_list = free_list_header();
      if (!free_list) {
         auto& header = m_data[m_size - 1];
         if (x == &header) {
            header.m_next_free = x;
            return;
         }
         else {
            size_t idx = x - m_data;
            base::emplace_back(nullptr);
            x = m_data + idx;
            free_list = free_list_header();
         }
      }
      assert(free_list);

      x->m_next_free = exchange(free_list->m_next_free, x);
   }
   cValuePtr pop_free() noexcept
   {
      if (auto free_list = free_list_header()) {
         auto& first = free_list->m_next_free;
         if (first != free_list) {
            auto retval = exchange(first, next_free(first));
            retval->m_child = nullptr;
            return retval.get();
         }
         first = m_data - 1; // mark as non-empty
      }
      return nullptr;
   }

   void add(cObject* x, cObject* parent)
   {
      auto& introspector_entry = desc();
      switch (introspector_entry.m_type) {
         case eRelationshipType::One2One: {
            assert(!m_parent_ref);
            if (m_object) {
               remove();
            }
            m_object = x;
            auto [rel_p, lock_p] = x->get_relationship(introspector_entry.m_id, true, true);
            rel_p->set_parent_ref(parent, 0);
         } break;

         case eRelationshipType::One2Many: {
            assert(!m_parent_ref);
            auto [rel_p, lock_p] = x->get_relationship(introspector_entry.m_id, true, true);
            auto [old_parent, old_parent_idx] = rel_p->parent();
            if (old_parent != parent) {
               if (old_parent) {
                  if (auto [old_rel, old_lock] = old_parent->get_relationship(introspector_entry.m_id); old_rel) {
                     old_rel->remove(old_parent_idx);
                  }
               }
               size_t pos;
               if (auto free = pop_free()) {
                  free->m_child = x;
                  pos = free - m_data;
               }
               else {
                  pos = m_size;
                  base::emplace_back(x);
               }
               rel_p->set_parent_ref(parent, pos);
            }
         } break;

         case eRelationshipType::Many2Many:
            break;
      }
   }

   void remove(size_t idx = 0)
   {
      auto& introspector_entry = desc();
      switch (introspector_entry.m_type) {
         case eRelationshipType::One2Many: {
            assert(!m_parent_ref);
            auto& x = m_data[idx];
            x.m_child->remove_relationship(introspector_entry.m_id, true);
            push_free(m_data + idx);
         } break;

         case eRelationshipType::One2One:
            assert(!m_parent_ref);
            m_object->remove_relationship(introspector_entry.m_id, true);
            m_object = nullptr;
            break;
      }
   }

   void set_parent_ref(cObject* parent, size_t pos)
   {
      m_parent_ref = true;
      m_parent_idx = int(pos);
      m_object = parent;
   }
};

} // namespace db
