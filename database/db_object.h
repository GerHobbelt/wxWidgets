#pragma once

#include <boost/intrusive/list.hpp>

#include "db_introspector.h"
#include "db_threading.h"

namespace db {

using namespace std;
namespace bin = boost::intrusive;

template <typename Traits>
class cObject : public bin::list_base_hook<>
{
protected:
   template <typename T>
   using alloc = typename Traits::template alloc<T>;

   template <typename T>
   using alloc_traits = allocator_traits<alloc<T>>;

   using cIntrospector = cIntrospector<Traits>;

   template <class T>
   using vector = typename cIntrospector::template vector<T>;

   using cObjDescs = typename decltype(cIntrospector::m_obj_desc);
   using cObjDesc = typename cObjDescs::value_type;

   using cPropDescs = typename decltype(cIntrospector::m_prop_desc);
   using cPropDesc = typename cPropDescs::value_type;

   using cRelDescs = typename decltype(cIntrospector::m_rel_desc);
   using cRelDesc = typename cRelDescs::value_type;

   using eRelationshipType = typename cIntrospector::eRelationshipType;
   using cRelationship = cRelationship<Traits>;

   using cRelationships = vector<cRelationship>;

public:
   using eObjId = decltype(cObjDesc::m_id);
   using ePropId = decltype(cPropDesc::m_id);
   using eRelId = decltype(cRelDesc::m_id);

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

   auto get_relationship(eRelId id, bool parent_ref = false) const
   {
      cLockGuard lg(m_lock);
      typename cRelationships::pointer retval = nullptr;

      auto it = find_if(m_relationships.begin(), m_relationships.end(), [id, parent_ref](cRelationship& rel) {
         if (rel.m_idx < 0) {
            return false;
         }
         auto rel_id = rel.desc().m_id;
         auto rel_parent_ref = !!get<0>(rel.parent());
         return rel_id == id && rel_parent_ref == parent_ref;
      });

      if (it != m_relationships.end()) {
         retval = it;
      }
      return make_pair(retval, move(lg));
   }
   auto get_relationship(eRelId id, bool parent_ref = false, bool create = false)
   {
      auto [rel, lock] = as_const(*this).get_relationship(id, parent_ref);
      if (!rel && create) {
         auto& relationships = Traits::introspector.m_rel_desc;
         auto b = begin(relationships), e = end(relationships);
         auto it_desc = find_if(b, e, [id](cRelDesc& desc) { return desc.m_id == id; });
         if (it_desc != e) {
            lock.upgrade();
            auto it = find_if(m_relationships.begin(), m_relationships.end(), [](cRelationship& rel) { return !rel.is_valid(); });
            if (it != m_relationships.end()) {
               m_relationships.emplace(it, it_desc - b);
               rel = it;
            }
            else {
               m_relationships.emplace_back(it_desc - b);
               rel = m_relationships.last();
            }
            lock.downgrade();
         }
      }
      return make_pair(rel, move(lock));
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

   void include(eRelId id, cObject& x)
   {
      auto [rel, lock] = get_relationship(id, false, true);
      assert(rel);
      rel->add(&x, this);
   }
   void exclude(eRelId id, cObject& x)
   {
      auto [rel, lock] = get_relationship(id);
      auto [rel_p, lock_p] = x.get_relationship(id, true);
      if (rel && rel_p) {
         auto [parent, parent_idx] = rel_p->parent();
         assert(parent);
         rel->remove(parent_idx);
      }
   }
   size_t count(eRelId id, bool parent_ref = false) const
   {
      if (auto [rel, lock] = get_relationship(id, parent_ref); rel) {
         return rel->count();
      }
      return 0;
   }
   cObject* parent(eRelId id) const
   {
      if (auto [rel, lock] = get_relationship(id, true); rel) {
         return rel->m_object;
      }
      return nullptr;
   }

   void on_destroy()
   {
   }

   template <typename T>
   void before_propmodify(ePropId id, T val)
   {
   }
   template <typename T>
   void after_propmodify(ePropId id, T val)
   {
   }

   template <typename T>
      requires derived_from<T, cObject>
   static auto factory()
   {
      alloc<T> a;
      auto new_p = a.allocate(1);
      alloc_traits<T>::template construct(a, new_p);
      return alloc_traits<cObject>::pointer(new_p);
   }

protected:
   eObjId m_type;
   mutable cRWLock<Traits> m_lock;
   cRelationships m_relationships;
};

template <typename Traits>
using cObjectPtr = typename allocator_traits<typename Traits::template alloc<cObject<Traits>>>::pointer; // cObject*

} // namespace db
