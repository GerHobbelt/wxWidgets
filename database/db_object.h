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

template <typename T, typename Traits>
struct cObjRelationshipsTraits
   : public cVectorTraits<T, typename cTypes<Traits>::template alloc<T>>
{
   using types = cTypes<Traits>;
   using base = cVectorTraits<T, typename cTypes<Traits>::template alloc<T>>;

   using alloc = typename base::alloc;
   using pointer = typename base::pointer;
   using size_type = uint8_t;

   struct cData
   {
      pointer m_data = nullptr;
      typename types::uid_t m_uid;
      typename types::eObjId m_type;
      union {
         size_type m_size = 0;
         alloc m_alloc;
      };

      cData(typename types::eObjId type)
         : m_type(type)
      {
      }
      cData(cData&& x)
         : m_size(exchange(x.m_size, 0))
         , m_data(exchange(x.m_data, nullptr))
         , m_type(x.m_type)
      {
      }
   };

   using vector = typename types::cIntrospector::template vector<T, cObjRelationshipsTraits>;
};

template <typename Traits>
class cObject
   : public bin::list_base_hook<>
   , public cObjRelationshipsTraits<cRelationship<Traits>, Traits>::vector
{
protected:
   using types = cTypes<Traits>;
   using cObjectPtr = typename types::cObjectPtr;

   template <typename U>
   using alloc_traits = typename types::template alloc_traits<U>;

   using eObjId = typename types::eObjId;
   using ePropId = typename types::ePropId;
   using eRelId = typename types::eRelId;

   using eRelationshipType = typename types::eRelationshipType;

   using uid_t = typename types::uid_t;

   using cRelationship = cRelationship<Traits>;
   using cRelationships = typename cObjRelationshipsTraits<cRelationship, Traits>::vector;
   using pointer = typename cRelationships::pointer;

   using cRelationships::m_type, cRelationships::m_uid;

public:
   cObject(eObjId type)
      : cRelationships(type)
   {
   }
   cObject(cObject&& x)
      : cRelationships(move(x))
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

   bool is_valid() const
   {
      return !!m_uid;
   }

   auto get_relationship(eRelId id, bool parent_ref = false) const
   {
      pointer retval = nullptr;

      auto it = find_if(cRelationships::begin(), cRelationships::end(), [id, parent_ref](cRelationship& rel) {
         if (!rel.is_valid()) {
            return false;
         }
         auto rel_id = rel.desc().m_id;
         if (rel_id != id) {
            return false;
         }
         auto rel_type = rel.desc().m_type;
         if (rel_type == eRelationshipType::Many2Many) {
            return true;
         }
         auto [parent, idx] = rel.parent();
         return !!parent == parent_ref;
      });

      if (it != cRelationships::end()) {
         retval = it;
      }
      return retval;
   }
   auto get_relationship(eRelId id, bool parent_ref = false, bool create = false)
   {
      auto rel = as_const(*this).get_relationship(id, parent_ref);
      if (!rel && create) {
         auto& relationships = Traits::introspector.m_rel_desc;
         auto b = begin(relationships), e = end(relationships);
         auto it_desc = find_if(b, e, [id](typename types::cRelDesc& desc) { return desc.m_id == id; });
         if (it_desc != e) {
            auto idx = typename cRelationship::size_type(it_desc - b);
            auto it = find_if(cRelationships::begin(), cRelationships::end(), [](cRelationship& rel) { return !rel.is_valid(); });
            if (it != cRelationships::end()) {
               cRelationships::emplace(it, idx);
               rel = it;
            }
            else {
               cRelationships::emplace_back(idx);
               rel = cRelationships::last();
            }
         }
      }
      return rel;
   }
   void remove_relationship(eRelId id, bool parent_ref = false)
   {
      cRelationship* retval = nullptr;

      auto it = find_if(cRelationships::begin(), cRelationships::end(), [id, parent_ref](cRelationship& rel) {
         return rel.desc().m_id == id && !!get<0>(rel.parent()) == parent_ref;
         });

      if (it != cRelationships::end()) {
         cRelationships::erase(it);
      }
   }
   void remove_all_relationships()
   {
      for (auto rel = cRelationships::begin(); rel != cRelationships::end(); ++rel) {
         rel->clear();
      }
      cRelationships::clear();
   }
   void include(eRelId id, cObject& x, bool parent_ref = false)
   {
      if (x.is_valid()) {
         auto rel = get_relationship(id, parent_ref, true);
         assert(rel);
         rel->add(&x, this);
      }
   }
   void exclude(eRelId id, bool parent_ref = false)
   {
      if (auto rel = get_relationship(id, parent_ref)) {
         if (auto child = rel->child()) {
            exclude(id, *child, *rel);
         }
      }
   }
   void exclude(eRelId id, cObject& x, bool parent_ref = false)
   {
      if (auto rel = get_relationship(id, parent_ref)) {
         exclude(id, x, *rel);
      }
   }
   void exclude(eRelId id, cObject& x, cRelationship& rel)
   {
      if (rel.desc().m_type != types::eRelationshipType::Many2Many) {
         if (auto rel_p = x.get_relationship(id, true)) {
            auto [parent, parent_idx] = rel_p->parent();
            assert(parent);
            rel.remove(parent_idx);
         }
         return;
      }
      rel.remove(x);
   }
   size_t count(typename types::eRelId id, bool parent_ref = false) const
   {
      if (auto rel = get_relationship(id, parent_ref)) {
         return rel->count();
      }
      return 0;
   }
   typename types::cObject* parent(typename types::eRelId id) const
   {
      if (auto rel = get_relationship(id, true)) {
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
      auto id = obj->type();
      typename types::template alloc<T> a;
      alloc_traits<T>::template destroy(a, obj);
      alloc_traits<T>::template construct(a, obj, id);
   }

   template <typename Traits, Object<Traits> P, Object<Traits> C>
   friend class cRelationshipConstIterator;
};

} // namespace db
