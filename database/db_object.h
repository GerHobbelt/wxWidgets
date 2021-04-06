#pragma once

#include <concepts>

#include <boost/intrusive/list.hpp>

#include "db_types.h"
#include "db_introspector.h"
#include "db_threading.h"

namespace db {

using namespace std;
namespace bin = boost::intrusive;

template <typename Traits>
class cObject;

template <typename T, typename Traits>
concept Object = derived_from<T, cObject<Traits>>;

template <typename Traits>
class cObject
   : public bin::list_base_hook<>
   , public cTypes<Traits>
{
protected:
   using types = cTypes<Traits>;
   using cObjectPtr = typename types::cObjectPtr;

   template <typename U>
   using alloc_traits = typename types::template alloc_traits<U>;

   template <class T>
   using vector = typename types::cIntrospector::template vector<T>;

   using eObjId = typename types::eObjId;
   using ePropId = typename types::ePropId;
   using eRelId = typename types::eRelId;

   using uid_t = typename types::uid_t;

   using cRelationship = cRelationship<Traits>;
   using cRelationships = vector<cRelationship>;

public:
   cObject(eObjId type)
      : m_type(type)
      , m_lock(this)
   {
   }
   cObject(cObject&& x)
      : m_relationships(move(x.m_relationships))
      , m_lock(this)
   {
   }

   template <typename T>
   T get_property(ePropId id)
   {
      return T();
   }

   auto type() const
   {
      return m_type;
   }


   uid_t uid()
   {
      return m_uid;
   }
   void set_uid(uid_t uid)
   {
      m_uid = uid;
   }

   cRelationships& relationships()
   {
      return m_relationships;
   }

   auto get_relationship(eRelId id) const
   {
      cLockGuard lg(m_lock);
      typename cRelationships::pointer retval = nullptr;

      auto it = find_if(m_relationships.begin(), m_relationships.end(), [id](cRelationship& rel) {
         if (!rel.is_valid()) {
            return false;
         }
         auto rel_id = rel.desc().m_id;
         return rel_id == id;
      });

      if (it != m_relationships.end()) {
         retval = it;
      }
      return tuple(retval, move(lg));
   }
   auto get_relationship(eRelId id, bool parent_ref = false, bool create = false)
   {
      auto [rel, lock] = as_const(*this).get_relationship(id);
      if (!rel && create) {
         auto& relationships = Traits::introspector.m_rel_desc;
         auto b = begin(relationships), e = end(relationships);
         auto it_desc = find_if(b, e, [id](typename types::cRelDesc& desc) { return desc.m_id == id; });
         if (it_desc != e) {
            lock.upgrade();
            auto idx = typename cRelationship::size_type(it_desc - b);
            auto it = find_if(m_relationships.begin(), m_relationships.end(), [](cRelationship& rel) { return !rel.is_valid(); });
            if (it != m_relationships.end()) {
               m_relationships.emplace(it, idx);
               rel = it;
            }
            else {
               m_relationships.emplace_back(idx);
               rel = m_relationships.last();
            }
            lock.downgrade();
         }
      }
      return tuple(rel, move(lock));
   }
   void remove_relationship(eRelId id, bool parent_ref = false)
   {
      cLockGuard lg(m_lock);
      cRelationship* retval = nullptr;

      auto it = find_if(m_relationships.begin(), m_relationships.end(), [id, parent_ref](cRelationship& rel) {
         return rel.desc().m_id == id && !!get<0>(rel.parent()) == parent_ref;
      });

      if (it != m_relationships.end()) {
         lg.upgrade();
         m_relationships.erase(it);
         lg.downgrade();
      }
   }
   void remove_all_relationships()
   {
      for (auto rel = m_relationships.begin(); rel != m_relationships.end(); ++rel) {
         rel->clear();
      }
      m_relationships.empty();
   }
   void include(eRelId id, cObject& x)
   {
      auto [rel, lock] = get_relationship(id, false, true);
      assert(rel);
      rel->add(&x, this);
   }
   void exclude(eRelId id)
   {
      if (auto [rel, lock] = get_relationship(id); rel) {
         if (auto child = rel->child()) {
            exclude(id, *child, *rel);
         }
      }
   }
   void exclude(eRelId id, cObject& x)
   {
      if (auto [rel, lock] = get_relationship(id); rel) {
         exclude(id, x, *rel);
      }
   }
   void exclude(eRelId id, cObject& x, cRelationship& rel)
   {
      if (rel.desc().m_type != types::eRelationshipType::Many2Many) {
         if (auto [rel_p, lock_p] = x.get_relationship(id, true); rel_p) {
            auto [parent, parent_idx] = rel_p->parent();
            assert(parent);
            rel.remove(parent_idx);
         }
         return;
      }
      rel.remove(x);
   }
   size_t count(typename types::eRelId id) const
   {
      if (auto [rel, lock] = get_relationship(id); rel) {
         return rel->count();
      }
      return 0;
   }
   typename types::cObject* parent(typename types::eRelId id) const
   {
      if (auto [rel, lock] = get_relationship(id); rel) {
         return get<0>(rel->parent());
      }
      return nullptr;
   }

   template <typename T>
   void before_propmodify(typename types::ePropId id, T val)
   {
   }
   template <typename T>
   void after_propmodify(typename types::ePropId id, T val)
   {
   }

   template <Object<Traits> T>
   static auto factory()
   {
      typename types::template alloc<T> a;
      auto new_p = a.allocate(1);

      alloc_traits<T>::template construct(a, new_p);
      return alloc_traits<cObject>::pointer(new_p);
   }
   template <Object<Traits> T>
   static void disposer(cObjectPtr obj)
   {
      typename types::template alloc<T> a;
      alloc_traits<T>::template destroy(a, obj);
      a.deallocate(alloc_traits<T>::pointer(obj), 1);
   }

protected:
   eObjId m_type;
   uid_t m_uid;
   mutable cRWLock<Traits> m_lock;
   cRelationships m_relationships;

   template <typename Traits, Object<Traits> P, Object<Traits> C>
   friend class cRelationshipConstIterator;
};

} // namespace db
