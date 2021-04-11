#pragma once

namespace db {

template <class Traits>
class cObject;

template <class Traits>
class cRelationship;

static constexpr size_t page_size = 128;

template <typename Traits>
struct cPageBase
{
   template <class T>
   using alloc = typename Traits::template alloc<T>;

   using cPtr = typename allocator_traits<alloc<cPageBase>>::pointer;
   cPtr m_next = nullptr, m_prev = nullptr;
};

template <typename Traits, typename T>
struct cPage : public cPageBase<Traits>
{
   T m_objects[page_size];
};

template <typename Traits>
struct cIntrospector
{
   template <class T>
   using alloc = typename Traits::template alloc<T>;

   template <class T>
   using pointer = typename allocator_traits<alloc<T>>::pointer;

   template <class T>
   using cVectorTraits = cVectorTraits<T, alloc<T>>;

   template <typename T, typename Traits = cVectorTraits<T>>
   using vector = vector<T, Traits>;

   enum class ePropertyType { boolean, integer, real, string, coord, point };
   enum class eRelationshipType { One2One, One2Many, Many2Many };

   using cObject = cObject<Traits>;

   using eObjId = typename Traits::eObjId;
   using ePropId = typename Traits::ePropId;
   using eRelId = typename Traits::eRelId;

   struct cObjDesc
   {
      string m_name;
      eObjId m_id;
      //uint16_t m_obj_size, m_obj_offset;

      using construct_t = void (*)(cObject*, typename Traits::uid_t);
      construct_t m_construct;

      using destruct_t = void (*)(cObject*);
      destruct_t m_destruct;

      using page_factory_t = tuple<cPageBase<Traits>*, cObject*> (*)();
      page_factory_t m_page_factory;

      using page_disposer_t = void (*)(pointer<cPageBase<Traits>>);
      page_disposer_t m_page_disposer;
   };

   using cPropValuePtr = char cObject::*;

   struct cPropDesc
   {
      string m_name;
      ePropId m_id;
      ePropertyType m_type;
      int m_ptr;
   };

   struct cRelDesc
   {
      string m_name;
      eRelationshipType m_type;
      eRelId m_id;
      eObjId m_parent_id, m_child_id;
   };

   const size_t find_obj_desc(eObjId id) const noexcept
   {
      auto end = m_obj_desc.end();
      auto it = lower_bound(m_obj_desc.begin(), end, id, [](auto& desc1, auto id) {
         return desc1.m_id < id;
         });
      if (it != end) {
         return &*it - &m_obj_desc.front();
      }
      return -1;
   }

   std::vector<cObjDesc> m_obj_desc;
   std::vector<cPropDesc> m_prop_desc;
   std::vector<cRelDesc> m_rel_desc;

   ~cIntrospector()
   {
   }
};

} // namespace db
