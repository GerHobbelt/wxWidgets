
#include "pch.h"

namespace fss = std::filesystem;

struct shm_init {
   shm_init() {
      shm::remove();
      shm::segment_name = "smartdrc.data";
      auto shared_mem_file = fss::path(shm::shared_directory) / shm::segment_name;
      shm::create();
   }
} s_shm_init;

int main(int argc, char** argv)
{
   using namespace boost::dll;
   for (auto name : { "geometry.dll", "allegro.rdr", "dxf.rdr" }) {
      try {
         auto dll_path = program_location().parent_path() / name;
         auto lib = new shared_library(dll_path);
      }
      catch (...) {
         //
      }
   }
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
