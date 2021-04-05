// dbtest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <pch.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <array>
#include <assert.h>

using namespace std;
namespace bin = boost::intrusive;

namespace db {

   using namespace std;

   template <typename T, typename A = allocator<T>>
   struct cVectorTraits
   {
      using alloc = A;
      using pointer = typename allocator_traits<alloc>::pointer;

      struct cData
      {
         size_t m_size = 0;
         pointer m_data = nullptr;

         cData()
         {
         }
         cData(cData&& x)
            : m_size(x.m_size)
            , m_data(exchange(x.m_data, nullptr))
         {
         }
         cData& operator = (cData&& x)
         {
            m_size = exchange(x.m_size, 0);
            m_data = exchange(x.m_data, nullptr);
         }
      };
   };

   //template <class T>
   //void swap(typename cVectorTraits<T>::cData& x, typename cVectorTraits<T>::cData& y)
   //{
   //   std::swap(x.m_size, y.m_size);
   //   std::swap(x.m_data, y.m_data);
   //}

   template <typename T, typename Traits = cVectorTraits<T>>
   struct vector : public Traits::cData
   {
      using base = typename Traits::cData;
      using alloc = typename Traits::alloc;
      using allocator_traits = typename allocator_traits<alloc>;
      using pointer = typename allocator_traits::pointer;
      using traits = Traits;
      using value_type = T;

      using base::m_data, base::m_size;

      template <typename U, typename Size>
      static U append(U& data, Size& size)
      {
         alloc a;
         auto p = a.allocate(size + 1);
         for (auto i = 0; i < size; ++i) {
            allocator_traits::template construct(a, p + i, move(data[i]));
         }
         auto old_data = exchange(data, p);
         for (auto i = 0; i < size; ++i) {
            allocator_traits::template destroy<T>(a, old_data + i);
         }
         a.deallocate(old_data, size);
         return p + size++;
      }

      vector() noexcept
      {
      }
      vector(vector&& x) noexcept
         : base(move(x))
      {
      }
      vector(base&& x) noexcept
         : base(move(x))
      {
      }
      vector(initializer_list<T> in)
      {
         alloc a;
         m_size = in.size();
         auto p = m_data = a.allocate(m_size);
         for (auto i : in) {
            allocator_traits::template construct(a, p++, i);
         }
      }
      ~vector() noexcept
      {
         alloc a;
         for (auto i = 0; i < m_size; ++i) {
            allocator_traits::template destroy<T>(a, m_data + i);
         }
         a.deallocate(m_data, m_size);
      }

      void push_back(T&& x)
      {
         allocator_traits::template construct(alloc(), append(m_data, m_size), forward<T>(x));
      }
      template <typename ...ARG>
      void emplace_back(ARG... arg)
      {
         alloc a;
         auto p = append(m_data, m_size);
         allocator_traits::template construct(a, p, arg...);
      }

      template <typename ...ARG>
      void emplace(pointer pos, ARG... arg)
      {
         alloc a;
         allocator_traits::template construct(a, pos, arg...);
      }

      void erase(pointer elem)
      {
         alloc a;
         assert(size_t(elem - m_data) < m_size);
         allocator_traits::template destroy(a, elem);
      }

      size_t size() const noexcept
      {
         return m_size;
      }

      T& operator [](size_t idx) noexcept
      {
         assert(idx < m_size);
         return m_data[idx];
      }

      pointer begin() noexcept
      {
         return m_data;
      }
      pointer end() noexcept
      {
         return m_data + m_size;
      }
      pointer first() noexcept
      {
         if (!m_data || !m_size) {
            return nullptr;
         }
         return m_data;
      }
      pointer last() noexcept
      {
         if (!m_data || !m_size) {
            return nullptr;
         }
         return m_data + m_size - 1;
      }
      T& back() noexcept
      {
         assert(m_size);
         return m_data[m_size - 1];
      }
   };

   template <class Traits>
   class cObject;

   template <class Traits>
   class cRelationship;

   template <typename DerivedTraits, template <typename> class A = allocator>
   struct cDatabaseTraits
   {
      //static_assert(derived_from<DerivedTraits, cDatabaseTraits<DerivedTraits>>);

      enum class eRelationshipType {
         One2One,
         One2Many,
         Many2Many
      };

      template <class T>
      using alloc = A<T>;

      template <class T>
      using pointer = typename allocator_traits<alloc<T>>::pointer;

      template <typename T>
      using vector = vector<T, cVectorTraits<T, alloc<T>>>;

      struct cIntrospector
      {
         using eObjId = typename DerivedTraits::eObjId;
         using ePropId = typename DerivedTraits::ePropId;
         using eRelId = typename DerivedTraits::eRelId;

         struct cObjDesc
         {
            string m_name;
            eObjId m_id;
            using factory_t = pointer<cObject<DerivedTraits>> (*)();
            factory_t m_factory;
         };

         struct cPropDesc
         {
            string m_name;
            ePropId m_id;
         };

         struct cRelDesc
         {
            string m_name;
            eRelationshipType m_type;
            eRelId m_id;
            eObjId m_parent_id, m_child_id;
         };

         vector<cObjDesc> m_obj_desc;
         vector<cPropDesc> m_prop_desc;
         vector<cRelDesc> m_rel_desc;
      };
   };

   template <class Traits>
   struct cRWLock //TBD
   {
      cRWLock(cObject<Traits>* obj)
      {
      }
      ~cRWLock()
      {
      }

      void lock()
      {
      }
      void unlock()
      {
      }
      void upgrade()
      {
      }
      void downgrade()
      {
      }
   };

   template <class T>
   struct cLockGuard
   {
      cLockGuard(cRWLock<T>& lock)
      {
      }
      ~cLockGuard()
      {
      }
      void upgrade()
      {
      }
      void downgrade()
      {
      }
   };

   template <typename Traits>
   class cObject : public bin::list_base_hook<>
   {
   protected:
      template <typename T>
      using alloc = typename Traits::template alloc<T>;

      template <typename T>
      using alloc_traits = allocator_traits<alloc<T>>;

      template <class T>
      using vector = typename Traits::template vector<T>;

      using cIntrospector = decltype(Traits::introspector);

      using cObjDescs = typename decltype(cIntrospector::m_obj_desc);
      using cObjDesc = typename cObjDescs::value_type;

      using cPropDescs = typename decltype(cIntrospector::m_prop_desc);
      using cPropDesc = typename cPropDescs::value_type;

      using cRelDescs = typename decltype(cIntrospector::m_rel_desc);
      using cRelDesc = typename cRelDescs::value_type;

      using eRelationshipType = typename Traits::eRelationshipType;
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

      auto get_relationship(eRelId id, bool parent_ref = false, bool create = false)
      {
         cLockGuard lg(m_lock);
         typename cRelationships::pointer retval = nullptr;

         auto it = find_if(m_relationships.begin(), m_relationships.end(), [id, parent_ref](cRelationship& rel) {
            auto rel_id = rel.desc().m_id;
            auto rel_parent_ref = !!get<0>(rel.parent());
            return rel_id == id && rel_parent_ref == parent_ref;
         });

         if (it != m_relationships.end()) {
            retval = it;
         }
         else if (create) {
            auto& relationships = Traits::introspector.m_rel_desc;
            auto b = begin(relationships), e = end(relationships);
            auto it_desc = find_if(b, e, [id](cRelDesc& desc) {
               return desc.m_id == id;
            });

            if (it_desc != e) {
               lg.upgrade();
               auto it = find_if(m_relationships.begin(), m_relationships.end(), [](cRelationship& rel) {
                  return !rel.is_valid();
               });
               if (it != m_relationships.end()) {
                  m_relationships.emplace(it, it_desc - b);
                  retval = it;
               }
               else {
                  m_relationships.emplace_back(it_desc - b);
                  retval = m_relationships.last();
               }
               lg.downgrade();
            }
         }

         return make_pair(retval, move(lg));
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
         rel->add(&x, this);
      }

      void exclude(eRelId id, cObject& x)
      {
         auto [rel, lock] = get_relationship(id, false, true);
         auto [rel_p, lock_p] = x.get_relationship(id, true, true);

         auto [parent, parent_idx] = rel_p->parent();
         assert(parent);

         rel->remove(parent_idx);
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
      cRWLock<Traits> m_lock;
      cRelationships m_relationships;
   };

   template <typename Traits>
   using cObjectPtr = typename allocator_traits<typename Traits::template alloc<cObject<Traits>>>::pointer; // cObject*

   template <typename Traits, typename Alloc = typename Traits::template alloc<cObjectPtr<Traits>>>
   struct cRelationshipTraits : public cVectorTraits<cObjectPtr<Traits>, Alloc>
   {
      using base = cVectorTraits<cObjectPtr<Traits>, Alloc>;

      using pointer = cObjectPtr<Traits>; // cObject*
      using pointer2 = typename allocator_traits<Alloc>::pointer; //cObject**

      using cObject = cObject<Traits>;

      struct cData
      {
         int16_t m_idx;
         bool m_parent_ref = false;
         union
         {
            int m_size;
            int m_parent_idx;
         };
         union
         {
            pointer2 m_data;
            pointer m_object;
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
   };

   template <typename Traits>
   class cRelationship : public vector<typename cRelationshipTraits<Traits>::pointer, cRelationshipTraits<Traits>>
   {
   protected:
      template <typename T>
      using alloc = typename Traits::template alloc<T>;

      template <typename T>
      using alloc_traits = allocator_traits<alloc<T>>;

      using cIntrospector = decltype(Traits::introspector);

      using cRelDescs = typename decltype(cIntrospector::m_rel_desc);
      using cRelDesc = typename cRelDescs::value_type;

      using eRelationshipType = typename Traits::eRelationshipType;

   public:
      using cRelationshipTraits = cRelationshipTraits<Traits>;

   protected:
      using cObject = typename cRelationshipTraits::cObject;
      using cObjectPtr = typename cRelationshipTraits::pointer;
      using cObjectPtr2 = typename cRelationshipTraits::pointer2;

      using base = vector<cObjectPtr, cRelationshipTraits>;

      using base::m_idx, base::m_parent_idx, base::m_object;
      using base::m_parent_ref, base::m_data, base::m_size;

   public:
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
            for (auto i = 0; i < m_size; ++i) {
               destroy_at(m_data + i);
            }
            alloc<cObject*>().deallocate(m_data, m_size);
            m_size = 0;
         }
         m_idx = -1;
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
               }
               break;

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
                     cObjectPtr2 p = nullptr;
                     auto end = m_data + m_size;
                     if (auto it = find_if(m_data, end, [](cObjectPtr& obj) { return !obj; }); it != end) {
                        p = it;
                        *p = x;
                     }
                     else {
                        base::emplace_back(x);
                        p = base::first();
                     }
                     auto pos = p - m_data;
                     rel_p->set_parent_ref(parent, pos);
                  }
               }
               break;

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
               x->remove_relationship(introspector_entry.m_id, true);
               x = nullptr;
            } break;

            case eRelationshipType::One2One:
               assert(!m_parent_ref);
               m_object->remove_relationship(introspector_entry.m_id, true);
               m_object = nullptr;
               break;
         }
      }

      bool is_valid() const noexcept
      {
         return m_idx >= 0;
      }

      cRelDesc& desc() const
      {
         return Traits::introspector.m_rel_desc[m_idx];
      }

      auto parent() const
      {
         return tuple(m_parent_ref ? m_object : nullptr, m_parent_idx);
      }
      void set_parent_ref(cObject* parent, size_t pos)
      {
         m_parent_ref = true;
         m_parent_idx = int(pos);
         m_object = parent;
      }
   };

   template <typename Traits>
   class cDatabase
   {
   protected:
      using cIntrospector = decltype(Traits::introspector);

      using cObjDescs = typename decltype(cIntrospector::m_obj_desc);
      using cObjDesc = typename cObjDescs::value_type;

      using cPropDescs = typename decltype(cIntrospector::m_prop_desc);
      using cPropDesc = typename cPropDescs::value_type;

      using cRelDescs = typename decltype(cIntrospector::m_rel_desc);
      using cRelDesc = typename cRelDescs::value_type;

      using eRelationshipType = typename Traits::eRelationshipType;

   public:
      using cObject = cObject<Traits>;
      using cRelationship = cRelationship<Traits>;

      using eObjId = decltype(cObjDesc::m_id);
      using ePropId = decltype(cPropDesc::m_id);
      using eRelId = decltype(cRelDesc::m_id);

      cObject* create(eObjId id)
      {
         cObject new_obj = nullptr;
         if (cObjDesc* objdesc = Traits::introspector.find_obj_desc(id)) {
            if (objdesc->m_factory) {
               new_obj = objdesc->m_factory();
               auto& obj_list = m_objects[size_t(id)];
               obj_list.push_back(new_obj);
            }
         }
         return new_obj;
      }

   protected:
      //array<cRelationship, Traits::nObjTypes> m_objects;
      array<bin::list<cObject>, Traits::nObjTypes> m_objects;
   };
} // namespace db

template <class T>
#if 1
using a = allocator<T>;
#else
using a = shm::alloc<T>;
#endif

struct cTestDbTraits
   : public db::cDatabaseTraits<cTestDbTraits, a>
{
   template <typename T>
   using alloc = typename cDatabaseTraits::alloc<T>;

   enum class eObjId {
      eObj_Net,
      eObj_Comp,
      eObj_Pin,
      eObj_PinPair,
      eObj_Pad,

      eCount
   };

   static const auto nObjTypes = (size_t)eObjId::eCount;

   enum class ePropId {

      eCount
   };

   enum class eRelId {
      eRel_Comp_Pin,
      eRel_Comp_PinPair,
      eRel_Net_Pin,
      eRel_Pin_Pad,
      eRel_Comp_PowerPin,

      eCount
   };

   using cObject = db::cObject<cTestDbTraits>;
   using cDatabase = db::cDatabase<cTestDbTraits>;

   using cIntrospector = typename cDatabaseTraits::cIntrospector;
   static cIntrospector introspector;

   struct cPin : public cObject
   {
      string m_name;

      cPin(const char* s = nullptr)
         : cObject(eObjId::eObj_Pin)
         , m_name(s)
      {
      }
   };

   struct cComp : public cObject
   {
      string m_name;

      cComp(const char* s = nullptr)
         : cObject(eObjId::eObj_Comp)
         , m_name(s)
      {
      }
      cComp(cComp&& x)
         : cObject(move(x))
      {
      }

      void include_pin(cPin& pin)
      {
         include(eRelId::eRel_Comp_Pin, pin);
      }
      void exclude_pin(cPin& pin)
      {
         exclude(eRelId::eRel_Comp_Pin, pin);
      }
   };
};

//auto x = &cTestDbTraits::cObject::factory<cTestDbTraits::cComp>;
//auto y = cTestDbTraits::cIntrospector::cObjDesc::factory_t();

#define OBJ_DESC(id) cIntrospector::cObjDesc{#id, eObjId::eObj_##id, &cObject::factory<c##id>}
#define REL_DESC(id, type, parent, child) cIntrospector::cRelDesc{#id, eRelationshipType::type, eRelId::eRel_##id, eObjId::parent, eObjId::child}

cTestDbTraits::cIntrospector cTestDbTraits::introspector = {
   {
//      OBJ_DESC(Net),
      OBJ_DESC(Comp),
      OBJ_DESC(Pin),
//      OBJ_DESC(PinPair),
//      OBJ_DESC(Pad)
   },{
   },{
      REL_DESC(Comp_Pin, One2Many, eObj_Comp, eObj_Pin),
      REL_DESC(Net_Pin, One2Many, eObj_Net, eObj_Pin),
      REL_DESC(Pin_Pad, One2Many, eObj_Pin, eObj_Pad),
      REL_DESC(Comp_PowerPin, One2One, eObj_Comp, eObj_Pin)
   }
};

#undef REL_DESC
#undef OBJ_DESC

int main()
{
   cTestDbTraits::cPin p1("U1.1"), p2("U1.2"), p3("U1.3");
   cTestDbTraits::cComp u1("U1"), u2("U2");

   u1.include_pin(p1);
   u1.include_pin(p2);
   u1.exclude_pin(p1);
   u1.include_pin(p3);
   u1.include(cTestDbTraits::eRelId::eRel_Comp_PowerPin, p1);
}
