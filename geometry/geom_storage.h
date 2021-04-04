#pragma once

namespace shm {

   inline struct remove
   {
      remove() { bi::shared_memory_object::remove("mshm"); }
      ~remove() { bi::shared_memory_object::remove("mshm"); }
   } mshm_remover;

   inline bi::managed_shared_memory mshm(bi::open_or_create, "mshm", size_t(1) << 28, (void*)(size_t(1) << 40));

   template <typename T>
   using allocator = bi::allocator<T, bi::managed_shared_memory::segment_manager>;

   template <typename T, class F>
   allocator<T> rebind_allocator(allocator<F>& x)
   {
      allocator<T> a(x.get_segment_manager());
      return a;
   }

   template <typename T, typename A, typename ...Args>
   T* construct(A alloc, Args... args)
   {
      auto shared = rebind_allocator<T>(alloc).allocate(sizeof T);
      return new (shared.get()) T(args...);
   }

   template <typename T, typename A>
   T* construct_array(A alloc, size_t size)
   {
      auto shared = rebind_allocator<T>(alloc).allocate(size * sizeof T);
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
   using basic_string = bi::basic_string<C, Traits, allocator<C>>;

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
