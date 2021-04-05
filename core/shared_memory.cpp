#include "pch.h"
#include "shared_memory.h"

namespace shm {
   std::string shared_directory;
   std::string segment_name = "mshm";

   bi::managed_shared_memory mshm;

   void create()
   {
      mshm = move(bi::managed_shared_memory(bi::open_or_create, segment_name.c_str(), mem_initial_size, (void*)mem_base));
   }
   void open()
   {
      mshm = move(bi::managed_shared_memory(bi::open_only, segment_name.c_str(), (void*)mem_base));
   }
   void remove()
   {
      mshm = move(bi::managed_shared_memory());
   }
}
