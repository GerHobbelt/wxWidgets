#pragma once

namespace db {

template <class Traits>
class cObject;

template <class Traits>
class cRelationship;

template <typename Traits>
struct cIntrospector
{
   template <class T>
   using alloc = typename Traits::template alloc<T>;

   template <class T>
   using pointer = typename allocator_traits<alloc<T>>::pointer;

   template <typename T>
   using vector = vector<T, cVectorTraits<T, alloc<T>>>;

   enum class ePropertyType { boolean, integer, real, string };
   enum class eRelationshipType { One2One, One2Many, Many2Many };

   using cObject = cObject<Traits>;

   using eObjId = typename Traits::eObjId;
   using ePropId = typename Traits::ePropId;
   using eRelId = typename Traits::eRelId;

   struct cObjDesc
   {
      string m_name;
      eObjId m_id;

      using factory_t = pointer<cObject> (*)();
      factory_t m_factory;

      using disposer_t = void (*)(pointer<cObject>);
      disposer_t m_disposer;
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

   pointer<cObjDesc> find_obj_desc(eObjId id) const noexcept
   {
      auto end = m_obj_desc.end();
      auto it = lower_bound(m_obj_desc.begin(), end, id, [](auto& desc1, auto id) {return desc1.m_id < id;});
      if (it != end) {
         return it;
      }
      return pointer<cObjDesc>(nullptr);
   }

   vector<cObjDesc> m_obj_desc;
   vector<cPropDesc> m_prop_desc;
   vector<cRelDesc> m_rel_desc;
};

} // namespace db
