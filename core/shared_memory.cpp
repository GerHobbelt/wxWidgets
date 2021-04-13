
#include "pch.h"
#include "shared_memory.h"

namespace shm {
   std::string shared_directory = ".";

   constexpr size_t max_seg = 256;
   static shared_memory shmem_pool[max_seg];

   shared_memory* segment_from_ptr(void* ptr)
   {
      auto p = (intptr_t)ptr;
      auto pseg = (p >> mem_base_shift) & (max_seg - 1);
      if (pseg > 0 && pseg <= max_seg) {
         return shmem_pool + pseg - 1;
      }
      assert(false);
      return nullptr;
   }
   shared_memory* create(const std::string& name)
   {
      for (size_t i = 0; i < max_seg; ++i) {
         auto &seg = shmem_pool[i];
         if (!seg.created) {
            seg.name = name;
            seg.base = (void *)(mem_base * ++i);
            (bi::managed_shared_memory&)seg = move(bi::managed_shared_memory(bi::open_or_create, name.c_str(), mem_initial_size, seg.base));
            seg.created = true;
            return &seg;
         }
      }
      assert(false);
      return nullptr;
   }
   void map_segment(shared_memory* s)
   {
      *(bi::managed_shared_memory *)s = move(bi::managed_shared_memory(bi::open_only, s->name.c_str(), s->base));
   }
   void unmap_segment(shared_memory* s)
   {
      *(bi::managed_shared_memory *)s = move(bi::managed_shared_memory());
   }
   void grow_segment(shared_memory* seg, size_t delta)
   {
      auto size = seg->get_size();

      unmap_segment(seg);

      auto size_delta = max(mem_initial_size, max(size / 4, delta));
      bi::managed_shared_memory::grow(seg->name.c_str(), size_delta);

      LOG("Growing shared memory buffer to {0}", size + size_delta);
      map_segment(seg);
   }
   void destroy(shared_memory* segment)
   {
      unmap_segment(segment);
      segment->created = false;
   }
}
