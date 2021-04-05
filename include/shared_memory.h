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
   class alloc
   {
   public:
      using value_type = T;
      using pointer = bi::offset_ptr<T>;
      using size_type = size_t;

      alloc()
      {
      }

      template<class T2>
      alloc(const alloc<T2>& other)
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

      pointer do_allocate(size_type count)
      {
         return pointer(static_cast<value_type*>(mshm.get_segment_manager()->allocate(count * sizeof(T))));
      }

      pointer allocate(size_type count)
      {
         try {
            grow_segment_if_low(count);
            return do_allocate(count);
         }
         catch (bi::bad_alloc) {
            grow_segment();
            try {
               return do_allocate(count);
            }
            catch (bi::bad_alloc) {
               assert(false);
               return nullptr;
            }
         }
      }

      pointer do_allocation_command(bi::allocation_type command, size_type limit_size, size_type& prefer_in_recvd_out_size, pointer& reuse)
      {
         value_type* reuse_raw = reuse.operator->();
         pointer const p = mshm.get_segment_manager()->allocation_command(command, limit_size, prefer_in_recvd_out_size, reuse_raw);
         reuse = reuse_raw;
         return p;
      }

      pointer allocation_command(bi::allocation_type command, size_type limit_size,
         size_type& prefer_in_recvd_out_size, pointer& reuse)
      {
         try {
            grow_segment_if_low(prefer_in_recvd_out_size);
            return do_allocation_command(command, limit_size, prefer_in_recvd_out_size, reuse);
         }
         catch (bi::bad_alloc) {
            grow_segment();
            try {
               return do_allocation_command(command, limit_size, prefer_in_recvd_out_size, reuse);
            }
            catch (bi::bad_alloc) {
               assert(false);
               return nullptr;
            }
         }
      }

      void deallocate(const pointer& ptr, size_type) 
      {
         mshm.get_segment_manager()->deallocate((void*)ptr.operator->());
      }

      template<class T2>
      struct rebind
      {
         typedef alloc<T2> other;
      };
   };

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
   using slist = bi::slist<T, alloc<T>>;

   template <typename T>
   using vector = bi::vector<T, alloc<T>>;

   template <typename K, typename V, typename C = less<K>>
   using map = bi::map<K, V, C, alloc<pair<const K, V>>>;

   template <typename C, class Traits = char_traits<C>>
   class basic_string : public bi::basic_string<C, Traits, alloc<C>>
   {
   public:
      using base = bi::basic_string<C, Traits, alloc<C>>;
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
   using string_map = bi::map<string, V, C, alloc<typename bi::map<string, V, C>::value_type>>;

} // namespace shm

namespace std {
   template <typename T>
   struct allocator_traits<shm::alloc<T>>
   {
      using pointer = shm::offset_ptr<T>;

      template<class U>
      using rebind_alloc = shm::alloc<U>;
      
      template <typename U, typename ...Args>
      static void construct(shm::alloc<U>& a, shm::offset_ptr<U> _Ptr, Args&& ...args)
      {
         //TBD
      }
      template <typename U>
      static void destroy(shm::alloc<U>& a, shm::offset_ptr<U> _Ptr)
      {
         //TBD
      };
   };
}