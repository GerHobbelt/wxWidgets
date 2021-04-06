#pragma once

#include "boost/interprocess/offset_ptr.hpp"

#include "db_types.h"
#include "db_vector.h"
#include "db_object.h"

namespace db {

using namespace std;
namespace bi = boost::interprocess;

template <typename Traits>
struct cRelationshipTraits
   : public cTypes<Traits>
{
   using types = cTypes<Traits>;
   using cObjectPtr = typename types::cObjectPtr;

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
      value_type(const cObjectPtr& child)
         : m_child(child)
      {
      }
      using free_ptr = bi::offset_ptr<value_type>;
      cObjectPtr m_child;
      free_ptr m_next_free;
   };

   using alloc = typename types::template alloc<value_type>;
   using pointer = typename types::template alloc_traits<value_type>::pointer;
   using free_pointer = typename value_type::free_ptr;
   using size_type = uint32_t;

   struct cData
   {
      using desc_idx_t = uint16_t;
      constexpr static auto invalid_desc_idx = (desc_idx_t)-1;

      desc_idx_t m_desc_idx;
      alloc m_alloc;
      bool m_parent_ref = false;
      union
      {
         size_type m_size;
         size_type m_parent_idx;
      };
      union
      {
         pointer m_data;
         cObjectPtr m_object;
      };

      cData(size_type rel_desc_idx)
         : m_desc_idx(desc_idx_t(rel_desc_idx))
         , m_data(nullptr)
         , m_size(0)
      {
      }
      cData(cData&& x)
         : m_desc_idx(exchange(x.m_desc_idx, invalid_desc_idx))
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
   static void move_n(pointer dest, pointer src, size_type size, alloc& a)
   {
      memcpy(dest, src, size * sizeof(value_type));
   }
   static void destroy(pointer pos, alloc& a)
   {
   }
   static void destroy_n(pointer pos, size_type size, alloc& a)
   {
      memset(pos, 0, size * sizeof(value_type));
   }

   using vector = vector<value_type, cRelationshipTraits>;
};

template <typename Traits>
class cRelationship
   : public cRelationshipTraits<Traits>::vector
   , public cTypes<Traits>
{
   using cRelationshipTraits = cRelationshipTraits<Traits>;
   using cValuePtr = typename cRelationshipTraits::pointer;
   using cFreePtr = typename cRelationshipTraits::free_pointer;
   using cObjectPtr = typename cRelationshipTraits::cObjectPtr;

   using types = cTypes<Traits>;
   using base = typename cRelationshipTraits::vector;
   using value_type = typename base::value_type;

#ifdef TESTING
public:
#endif
   using base::m_desc_idx, base::m_parent_idx, base::m_object;
   using base::m_parent_ref, base::m_data, base::m_size;
   using base::size;

public:
   using size_type = typename base::size_type;
   constexpr static size_type invalid_offset = -1;
   constexpr static auto invalid_desc_idx = base::invalid_desc_idx;

   cRelationship(size_type idx = invalid_desc_idx) noexcept
      : base(cRelationshipTraits::cData(idx))
   {
   }
   cRelationship(cRelationship&& x) noexcept
      : base(move(x))
   {
   }
   ~cRelationship() noexcept
   {
      if (is_valid()) {
         if (m_parent_ref || desc().m_type == types::eRelationshipType::One2One) {
            m_parent_idx = 0;
            m_object = nullptr;
            m_parent_ref = 0;
         }
         else {
            if (0/*TBD: strong rel*/) {
               for (auto i = m_size; i > 0; --i) {
                  //base::destroy(m_data + i);
               }
            }
         }
         m_desc_idx = invalid_desc_idx;
      }
   }

   value_type& at(size_type idx) noexcept = delete;
   const value_type& at(size_type idx) const noexcept = delete;
   value_type& operator[](size_type idx) noexcept = delete;
   const value_type& operator[](size_type idx) const noexcept = delete;

   bool is_valid() const noexcept
   {
      return m_desc_idx != invalid_desc_idx;
   }
   size_type count() const noexcept
   {
      if (m_parent_ref) {
         return m_object ? 1 : 0;
      }
      switch (desc().m_type)
      {
         case types::eRelationshipType::One2One:
            return m_object ? 1 : 0;

         case types::eRelationshipType::One2Many:
         case types::eRelationshipType::Many2Many:
            auto retval = m_size;
            for (auto free = first_free(); free; free = next_free(free)) {
               --retval;
            }
            return retval;
      }
      return 0;
   }
   void resize(size_type new_size)
   {
      auto old_size = m_size;
      auto old_data = m_data;
      auto old_free_list = free_list_header();

      auto delta = new_size - old_size;
      base::resize(new_size);

      // we assume that new items are zero initialized, and thus the free list is now valid and empty
      for (auto i = new_size; i > old_size;) {
         push_free(m_data + --i);
      }
      if (old_free_list) {
         // splice the old list with the new one
         auto old_idx = old_free_list - old_data;
         push_free(m_data + old_idx);
      }
   }

   auto desc_idx() const noexcept
   {
      return m_desc_idx;
   }
   types::cRelDesc& desc() const
   {
      assert(is_valid());
      return Traits::introspector.m_rel_desc[m_desc_idx];
   }

   auto parent(cRelationship* rel_p = nullptr) const
   {
      cObjectPtr invalid_object = nullptr;
      auto& introspector_entry = desc();
      switch (introspector_entry.m_type) {
         case types::eRelationshipType::One2One:
            return tuple(m_object, invalid_offset);

         case types::eRelationshipType::One2Many:
            if (m_parent_ref) {
               return tuple(m_object, m_parent_idx);
            }
            break;

         case types::eRelationshipType::Many2Many:
            {
               assert(rel_p);
               auto it = find_if(m_data, m_data + m_size, [this, rel_p, &introspector_entry](value_type& val) {
                     if (!is_free(val)) {
                        auto id = introspector_entry.m_id;
                        auto& child_rels = val.m_child->relationships();
                        auto it_p = find_if(child_rels.begin(), child_rels.end(), [id](cRelationship& rel) {
                           if (!rel.is_valid()) {
                              return false;
                           }
                           auto rel_id = rel.desc().m_id;
                           return rel_id == id;
                        });
                        if (it_p == rel_p) {
                           return true;
                        }
                     }
                     return false;
                  });
               if (it != m_data + m_size) {
                  auto idx = size_type(it - m_data);
                  return tuple(it->m_child, idx);
               }
            }
            break;
      }
      return tuple(invalid_object, invalid_offset);
   }
   auto child() const
   {
      return !m_parent_ref ? m_object : nullptr;
   }

   void add(types::cObject* x, types::cObject* parent)
   {
      auto& introspector_entry = desc();
      switch (introspector_entry.m_type) {
         case types::eRelationshipType::One2One:
            {
               assert(!m_parent_ref);
               if (m_object) {
                  remove();
               }
               m_object = x;
               auto [rel_p, lock_p] = x->get_relationship(introspector_entry.m_id, true, true);
               rel_p->set_parent_ref(parent, 0);
            } break;

         case types::eRelationshipType::One2Many:
            {
               assert(!m_parent_ref);
               auto [rel_p, lock_p] = x->get_relationship(introspector_entry.m_id, true, true);
               auto [old_parent, old_parent_idx] = rel_p->parent();
               if (old_parent != parent) {
                  if (old_parent) {
                     if (auto [old_rel, old_lock] = old_parent->get_relationship(introspector_entry.m_id); old_rel) {
                        old_rel->push_free(old_rel->m_data + old_parent_idx);
                     }
                  }
                  size_type pos;
                  if (auto free = pop_free()) {
                     free->m_child = x;
                     pos = size_type(free - m_data);
                  }
                  else {
                     pos = m_size;
                     base::emplace_back(x);
                  }
                  rel_p->set_parent_ref(parent, pos);
               }
            } break;

         case types::eRelationshipType::Many2Many:
            {
               assert(!m_parent_ref);
               auto [rel_p, lock_p] = x->get_relationship(introspector_entry.m_id, false, true);
               if (auto free = rel_p->pop_free()) {
                  free->m_child = parent;
               }
               else {
                  rel_p->base::emplace_back(parent);
               }
               if (auto free = pop_free()) {
                  free->m_child = x;
               }
               else {
                  base::emplace_back(x);
               }
            } break;
      }
   }

   void remove(types::cObject& x)
   {
      auto& introspector_entry = desc();
      if (introspector_entry.m_type == types::eRelationshipType::One2One) {
         if (m_object == &x) {
            remove();
         }
      }
      else if (auto idx = find(x); idx != invalid_desc_idx) {
         remove(idx);
      }
   }
   void remove(size_type idx = 0)
   {
      auto& introspector_entry = desc();
      switch (introspector_entry.m_type) {
         case types::eRelationshipType::Many2Many:
            if (!is_free(idx)) {
               assert(!m_parent_ref);
               auto& x = m_data[idx];
               if (auto [rel_p, lock_p] = x.m_child->get_relationship(introspector_entry.m_id); rel_p) {
                  auto [obj, idx] = rel_p->parent(this);
                  if (idx != invalid_offset) {
                     rel_p->push_free(rel_p->m_data + idx);
                  }
               }
               push_free(m_data + idx);
            }
            break;

         case types::eRelationshipType::One2Many:
            if (!is_free(idx)) {
               assert(!m_parent_ref);
               auto& x = m_data[idx];
               x.m_child->remove_relationship(introspector_entry.m_id, true);
               push_free(m_data + idx);
            }
            break;

         case types::eRelationshipType::One2One:
            assert(!m_parent_ref);
            m_object->remove_relationship(introspector_entry.m_id, true);
            m_object = nullptr;
            break;
      }
   }

   void clear()
   {
      if (m_parent_ref) {
         auto& introspector_entry = desc();
         switch (introspector_entry.m_type) {
            case types::eRelationshipType::One2One:
            case types::eRelationshipType::One2Many:
               if (auto [rel, lock] = m_object->get_relationship(introspector_entry.m_id); rel) {
                  rel->remove(m_parent_idx); // destroys *this
               }
               break;
         }
         return;
      }
      for (size_type idx = 0; idx != size(); ++idx) {
         remove(idx);
      }
   }

   void set_parent_ref(types::cObject* parent, size_type pos)
   {
      m_parent_ref = true;
      m_parent_idx = int(pos);
      m_object = parent;
   }

#ifndef TESTING
protected:
#endif
   size_type find(types::cObject& x)
   {
      auto it = find_if(m_data, m_data + m_size, [this, &x](value_type& val) {
         if (is_free(val)) {
            return false;
         }
         return val.m_child == &x;
      });
      if (it != m_data + m_size) {
         return size_type(it - m_data);
      }
      return invalid_desc_idx;
   }

   bool is_free(const value_type& x) const
   {
      auto p = x.m_next_free.get();
      auto offset = p - m_data;
      return p && size_t(offset) < m_size;
   }
   bool is_free(const size_type& idx) const
   {
      assert(idx >= 0 && idx < m_size);
      return is_free(m_data[idx]);
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
      auto header = free_list_header();
      if (cur != header) {
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
         if (auto header = base::last(); x == header) {
            header->m_next_free = x;
            return;
         }
         size_t idx = x - m_data;
         base::emplace_back(nullptr);
         x = m_data + idx;
         free_list = free_list_header();
      }
      assert(free_list);

      if (x != free_list) {
         x->m_next_free = exchange(free_list->m_next_free, x);
      }
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
         return free_list.get();
      }
      return nullptr;
   }
};

template <typename Traits, Object<Traits> P, Object<Traits> C>
class cRelationshipConstIterator : public cTypes<Traits>
{
   using types = cTypes<Traits>;
   using size_type = typename cRelationshipTraits<Traits>::size_type;

   const P* m_parent = nullptr;
   types::eRelId m_rel_id = types::eRelId(-1);
   size_type m_relidx = 0, m_index = -1;

public:
   cRelationshipConstIterator() = default;
   cRelationshipConstIterator(const cRelationshipConstIterator& x) = default;
   cRelationshipConstIterator(const cRelationshipConstIterator& x, size_type index)
      : cRelationshipConstIterator(x)
   {
      m_index = index;
   }
   cRelationshipConstIterator(const P* parent, types::eRelId rel_id)
      : m_parent(parent)
      , m_rel_id(rel_id)
   {
      reset();
   }

   void reset()
   {
      if (auto rel = get_relationship()) {
         m_relidx = size_type(rel - m_parent->m_relationships.m_data);
         if (rel->size()) {
            m_index = 0;
            if (rel->is_free(m_index)) {
               operator ++ ();
            }
         }
      }
   }

   bool operator == (const cRelationshipConstIterator& x) const
   {
      return m_parent == x.m_parent && m_rel_id == x.m_rel_id && m_index == x.m_index;
   }
   bool operator != (const cRelationshipConstIterator& x) const
   {
      return !operator == (x);
   }
   cRelationshipConstIterator& operator ++ ()
   {
      if (auto rel = get_relationship()) {
         while (m_index != -1 && ++m_index < rel->size()) {
            if (!rel->is_free(m_index)) {
               break;
            }
         }
         if (m_index == rel->size()) {
            m_index = -1;
         }
      }
      return *this;
   }

   const C* operator->() const
   {
      if (auto rel = get_relationship()) {
         if (m_index < rel->size()) {
            assert(!rel->is_free(m_index));
            auto& retval = rel->m_data[m_index];
            return (C*)retval.m_child;
         }
      }
      return nullptr;
   }
   const C& operator*() const
   {
      return *operator->();
   }

protected:
   cRelationship<Traits>* get_relationship() const
   {
      if (m_parent) {
         return get<0>(m_parent->get_relationship(m_rel_id));
      }
      return nullptr;
   }
};

template <typename Traits, Object<Traits> P, Object<Traits> C>
class cRelationshipIterator : public cRelationshipConstIterator<Traits, P, C>
{
   using base = cRelationshipConstIterator<Traits, P, C>;

public:
   using base::base, base::operator=, base::operator++;

   C* operator->() {return const_cast<C*>(base::operator->());}
   C& operator*() {return const_cast<C&>(base::operator*());}
};

template <typename Traits, Object<Traits> P, Object<Traits> C>
class cRelIterConstRange : public cTypes<Traits>
{
   using types = cTypes<Traits>;
   using cRelationshipConstIterator = cRelationshipConstIterator<Traits, P, C>;

protected:
   cRelationshipConstIterator m_beg;

public:
   cRelIterConstRange(const P* parent, types::eRelId rel_id)
      : m_beg(parent, rel_id)
   {
   }

   cRelationshipConstIterator begin() const
   {
      return m_beg;
   }
   cRelationshipConstIterator end() const
   {
      return cRelationshipIterator(m_beg, -1);
   }
};

template <typename Traits, Object<Traits> P, Object<Traits> C>
class cRelIterRange : public cRelIterConstRange<Traits, P, C>
{
   using base = cRelIterConstRange<Traits, P, C>;
   using cRelationshipIterator = cRelationshipIterator<Traits, P, C>;

public:
   using base::base;

   auto begin()
   {
      return (cRelationshipIterator&)base::m_beg;
   }
   auto end()
   {
      return cRelationshipIterator(base::m_beg, -1);
   }
};

} // namespace db
