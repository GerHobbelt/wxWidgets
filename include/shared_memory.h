#pragma once

#include "logger.h"

namespace shm {

   inline auto segment_name = "mshm";

   inline struct remove
   {
      remove() { bi::shared_memory_object::remove(segment_name); }
      ~remove() { bi::shared_memory_object::remove(segment_name); }
   } mshm_remover;

   inline constexpr auto mem_base = 1ui64 << 40; // 1TB boundary
   inline constexpr auto mem_initial_size = 1ui64 << 23; // 8M initial size
   inline bi::managed_shared_memory mshm(bi::open_or_create, segment_name, mem_initial_size, (void*)mem_base);

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

         {auto tmp = move(mshm);} // unmap the segment

         auto size_delta = std::max(mem_initial_size, std::max(size / 4, delta));
         bi::managed_shared_memory::grow(segment_name, size_delta);

         LOG("Growing shared memory buffer to {0}", size + size_delta);
         mshm = move(bi::managed_shared_memory(bi::open_only, segment_name, (void*)mem_base));
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

   inline allocator<> s_alloc;

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

   template <typename C, class Traits = std::char_traits<C>>
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
