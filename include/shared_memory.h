#pragma once

#include <string>

#include "logger.h"
#include "assert_mgr.h"

namespace shm {
   using namespace std;

   extern CORE_API string shared_directory;
   extern CORE_API string segment_name;
}

#define BOOST_INTERPROCESS_SHARED_DIR_FUNC

namespace boost::interprocess::ipcdetail {
   void get_shared_dir(std::string& shared_dir)
   {
      shared_dir = shm::shared_directory;
   }
}

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/slist.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/set.hpp>

namespace bi = boost::interprocess;

namespace shm {

   using namespace std;

   inline constexpr auto mem_base = 1ui64 << 40; // 1TB boundary
   inline constexpr auto mem_initial_size = 1ui64 << 23; // 8M initial size

   extern CORE_API bi::managed_shared_memory mshm;

   CORE_API void create();
   CORE_API void open();
   CORE_API void remove();

   template <class T = char>
   class allocator : public bi::allocator<T, bi::managed_shared_memory::segment_manager>
   {
      using base = bi::allocator<T, bi::managed_shared_memory::segment_manager>;
   public:

      allocator()
         : base(mshm.get_segment_manager())
      {
      }

      void grow_segment(size_t delta = 0)
      {
         auto size = mshm.get_size();

         remove(); // unmap the segment

         auto size_delta = max(mem_initial_size, max(size / 4, delta));
         bi::managed_shared_memory::grow(segment_name.c_str(), size_delta);

         LOG("Growing shared memory buffer to {0}", size + size_delta);
         open();
      }
      
      void grow_segment_if_low(size_t delta)
      {
         auto delta1 = delta + mem_initial_size / 32;
         auto free_space = mshm.get_free_memory();
         if (free_space <= delta1) {
            grow_segment(delta1);
         }
      }

      typename base::pointer allocate(typename base::size_type count)
      {
         try {
            grow_segment_if_low(count);
            return base::allocate(count);
         }
         catch (bi::bad_alloc) {
            grow_segment();
            try {
               return base::allocate(count);
            }
            catch (bi::bad_alloc) {
               assert(false);
               return nullptr;
            }
         }
      }
      typename base::pointer allocation_command(bi::allocation_type command, typename base::size_type limit_size,
            typename base::size_type& prefer_in_recvd_out_size, typename base::pointer& reuse)
      {
         try {
            grow_segment_if_low(prefer_in_recvd_out_size);
            return base::allocation_command(command, limit_size, prefer_in_recvd_out_size, reuse);
         }
         catch (bi::bad_alloc) {
            grow_segment();
            try {
               return base::allocation_command(command, limit_size, prefer_in_recvd_out_size, reuse);
            }
            catch (bi::bad_alloc) {
               assert(false);
               return nullptr;
            }
         }
      }
   };

   template <typename T>
   allocator<T> alloc()
   {
      return allocator<T>();
   }

   template <typename T, typename ...Args>
   T* construct(Args... args)
   {
      auto shared = alloc<T>().allocate(1);
      return new (shared.get()) T(args...);
   }

   template <typename T>
   T* construct_array(size_t size)
   {
      auto shared = alloc<T>().allocate(size);
      return new (shared.get()) T[size];
   }

   template <typename T>
   using offset_ptr = bi::offset_ptr<T>;

   template <typename T>
   using unique_offset_ptr = bi::offset_ptr<T>;//typename bi::managed_unique_ptr<T, bi::managed_shared_memory>::type;

   template <typename T, typename A, typename ...Args>
   auto make_unique_offset_ptr(A alloc, Args... args)
   {
      //return bi::make_managed_unique_ptr<T, bi::managed_shared_memory>(shared_construct<T>(alloc, args...), mshm);
      return shared_construct<T>(alloc, args...);
   }

   template <typename T>
   using slist = bi::slist<T, allocator<T>>;

   template <typename T>
   using vector = bi::vector<T, allocator<T>>;

   template <typename K, typename V, typename C = less<K>>
   using map = bi::map<K, V, C, allocator<pair<const K, V>>>;

   template <typename C, class Traits = char_traits<C>>
   class basic_string : public bi::basic_string<C, Traits, allocator<C>>
   {
   public:
      using base = bi::basic_string<C, Traits, allocator<C>>;
      using base::base;
   };

   using string = basic_string<char>;

   struct string_less
   {
      using is_transparent = true_type;

      template <typename K>
      bool operator()(const string& s, const K& k) const
      {
         return s.compare(k) < 0;
      }
      template <typename K, typename = enable_if_t<!is_same_v<K, string>>>
      bool operator()(const K& k, const string& s) const
      {
         return s.compare(k) > 0;
      }
   };

   template <typename V, typename C = string_less>
   using string_map = bi::map<string, V, C, allocator<pair<const string, V>>>;

} // namespace shm
