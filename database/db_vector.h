
#pragma once

#include <memory>

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
      alloc m_alloc;

      cData()
      {
      }
      cData(cData&& x)
         : m_size(x.m_size)
         , m_data(exchange(x.m_data, nullptr))
      {
      }
      cData& operator=(cData&& x)
      {
         m_size = exchange(x.m_size, 0);
         m_data = exchange(x.m_data, nullptr);
      }
   };

   template <typename... ARG>
   static void construct(pointer pos, alloc& a, ARG&&... args)
   {
      allocator_traits<alloc>::template construct(a, pos, forward<ARG>(args)...);
   }
   static void move_n(pointer dest, pointer src, size_t size, alloc& a)
   {
      for (auto i = 0; i < size; ++i) {
         construct(dest + i, a, move(src[i]));
      }
   }
   static void destroy(pointer pos, alloc& a)
   {
      allocator_traits<alloc>::template destroy(a, pos);
   }
   static void destroy_n(pointer pos, size_t size, alloc& a)
   {
      for (auto i = 0; i < size; ++i) {
         destroy(pos++, a);
      }
   }
};

//template <class T>
//void swap(typename cVectorTraits<T>::cData& x, typename cVectorTraits<T>::cData& y)
//{
//   std::swap(x.m_size, y.m_size);
//   std::swap(x.m_data, y.m_data);
//}

template <typename T, typename Traits = cVectorTraits<T>>
class vector : public Traits::cData
{
public:
   using base = typename Traits::cData;
   using alloc = typename Traits::alloc;
   using allocator_traits = typename allocator_traits<alloc>;
   using pointer = typename allocator_traits::pointer;
   using traits = Traits;
   using value_type = T;

   using base::m_data, base::m_size;

protected:
   auto append(int size = 1)
   {
      auto p = this->m_alloc.allocate(m_size + size);
      Traits::move_n(p, m_data, m_size, this->m_alloc);
      auto old_data = exchange(m_data, p);
      Traits::destroy_n(old_data, m_size, this->m_alloc);
      this->m_alloc.deallocate(old_data, m_size);
      p += m_size;
      m_size += size;
      return p;
   }

public:
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
      m_size = in.size();
      auto p = m_data = this->m_alloc.allocate(m_size);
      for (auto i: in) {
         Traits::construct(p++, this->m_alloc, i);
      }
   }
   ~vector() noexcept
   {
      Traits::destroy_n(m_data, m_size, this->m_alloc);
      this->m_alloc.deallocate(m_data, m_size);
   }

   void resize(int size)
   {
      auto delta = size - m_size;
      if (delta > 0) {
         auto p = append(delta);
         for (size_t i = 0; i < delta; ++i) {
            Traits::construct(p++, this->m_alloc);
         }
      }
   }

   void push_back(T&& x)
   {
      auto p = append();
      Traits::construct(p, this->m_alloc, forward<T>(x));
   }
   template <typename... ARG>
   void emplace_back(ARG... args)
   {
      auto p = append();
      Traits::construct(p, this->m_alloc, forward<ARG>(args)...);
   }

   template <typename... ARG>
   void emplace(pointer pos, ARG... args)
   {
      Traits::construct(pos, this->m_alloc, forward<ARG>(args)...);
   }

   void erase(pointer elem)
   {
      assert(size_t(elem - m_data) < m_size);
      Traits::destroy(elem, this->m_alloc);
   }

   size_t size() const noexcept
   {
      return m_size;
   }

   const T& operator[](size_t idx) const noexcept
   {
      assert(idx < m_size);
      return m_data[idx];
   }
   T& operator[](size_t idx) noexcept
   {
      assert(idx < m_size);
      return m_data[idx];
   }

   const pointer begin() const noexcept
   {
      return m_data;
   }
   pointer begin() noexcept
   {
      return m_data;
   }
   const pointer end() const noexcept
   {
      return m_data + m_size;
   }
   pointer end() noexcept
   {
      return m_data + m_size;
   }
   const pointer first() const noexcept
   {
      if (!m_data || !m_size) {
         return nullptr;
      }
      return m_data;
   }
   pointer first() noexcept
   {
      if (!m_data || !m_size) {
         return nullptr;
      }
      return m_data;
   }
   const pointer last() const noexcept
   {
      if (!m_data || !m_size) {
         return nullptr;
      }
      return m_data + m_size - 1;
   }
   pointer last() noexcept
   {
      if (!m_data || !m_size) {
         return nullptr;
      }
      return m_data + m_size - 1;
   }
};

} // namespace db
