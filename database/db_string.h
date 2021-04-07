
#pragma once

#include <memory>

namespace db {

using namespace std;

template <typename T, typename A = allocator<T>>
struct cStringTraits
{
   using value_type = T;
   using alloc = typename allocator_traits<A>::template rebind_alloc<value_type>;
   using allocator_traits = typename allocator_traits<alloc>;
   using pointer = typename allocator_traits::pointer;
   using size_type = size_t;

   struct cData
   {
      static inline T empty_string[] = { T(0) };
      union {
         pointer m_data = empty_string;
         alloc m_alloc;
      };

      cData()
      {
      }
      cData(cData&& x)
         : m_data(exchange(x.m_data, empty_string))
      {
      }
      cData& operator=(cData&& x)
      {
         m_data = exchange(x.m_data, empty_string);
      }

      auto operator<=>(const cData& x) const noexcept = default;
   };
};

template <typename T, typename Traits = cStringTraits<T>>
class string : public Traits::cData
{
public:
   using traits = Traits;
   using base = typename Traits::cData;
   using alloc = typename Traits::alloc;
   using allocator_traits = typename Traits::allocator_traits;
   using pointer = typename Traits::pointer;
   using value_type = typename Traits::value_type;
   using size_type = typename Traits::size_type;

   using base::m_data, base::m_alloc;

public:
   string() noexcept
   {
   }
   string(const string& x) noexcept
      : string(x.m_data)
   {
   }
   string(string&& x) noexcept
      : base(move(x))
   {
   }
   string(const value_type* in) noexcept
   {
      if (size_type size = length(in)) {
         m_data = m_alloc.allocate(size + 1);
         copy(in, in + size, m_data);
         m_data[size] = 0;
      }
   }
   ~string() noexcept
   {
      clear();
   }

   string& operator=(const string& x) noexcept
   {
      return operator=(x.m_data);
   }
   string& operator=(const value_type* x) noexcept
   {
      clear();
      if (size_type size = length(x)) {
         m_data = m_alloc.allocate(size + 1);
         copy(x, x + size, m_data);
         m_data[size] = 0;
      }
      return *this;
   }
   string& operator=(string&& x) noexcept
   {
      clear();
      swap(m_data, x.m_data);
      return *this;
   }

   auto operator<=>(const value_type* x) const noexcept
   {
      if (!*m_data) {
         return (x && *x) ? strong_ordering::less : strong_ordering::equal;
      }
      if (!x || !*x) {
         return strong_ordering::greater;
      }
      auto len = length(), x_len = length(x);
      auto minlen = min(len, x_len);
      auto s = begin();
      auto it = mismatch(s, s + minlen, x);
      if (it.first == s + minlen) {
         return len <=> x_len;
      }
      return *it.first <=> *it.second;
   }
   auto operator<=>(const string& x) const noexcept
   {
      return *this <=> x.m_data;
   }
   bool operator==(const value_type* x) const
   {
      return (*this <=> x) == 0;
   }
   bool operator==(const string& x) const
   {
      return *this == x.m_data;
   }

   bool empty() const noexcept
   {
      return !*m_data;
   }

   void clear() noexcept
   {
      if (*m_data) {
         auto size = length() + 1;
         m_alloc.deallocate(m_data, size);
         m_data = base::empty_string;
      }
   }

   static constexpr size_type length(const value_type * in) noexcept
   {
      size_type size = 0;
      if (in) {
         auto s = in;
         while (*s) {
            ++s;
         }
         size = size_type(s - in);
      }
      return size;
   }
   size_type length() const noexcept
   {
      return length(m_data);
   }

   const value_type* c_str() const noexcept
   {
      return m_data;
   }

   const value_type& at(size_type idx) const noexcept
   {
      return m_data[idx];
   }

   const value_type& operator[](size_type idx) const noexcept
   {
      return at(idx);
   }

   const pointer begin() const noexcept
   {
      return m_data;
   }
   const pointer end() const noexcept
   {
      return m_data + length();
   }
   const pointer first() const noexcept
   {
      return m_data;
   }
   const pointer last() const noexcept
   {
      return m_data + length() - 1;
   }
};

} // namespace db
