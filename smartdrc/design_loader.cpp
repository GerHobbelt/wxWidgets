
#include "pch.h"

#include "logger.h"
#include "pcb_loader.h"
#include "shared_memory.h"
#include "database.h"
#include "smartdrc.h"

using namespace std;
using namespace boost::dll;
using path = filesystem::path;

void clear_design(cDatabase* db)
{
   if (geom::iEngine* ge = db->geom_engine()) {
//      ge->clear();
   }
}

namespace fss = filesystem;

void load_design(const fss::path& fname, cDatabase * db)
{
   static map<path, path> s_loader_map{
      {".exb", "allegro"},
      {".dxf", "dxf"},
   };
   if (auto it = s_loader_map.find(fname.extension()); it != s_loader_map.end()) {
      auto rdr_path = program_location().parent_path() / (it->second.string() + ".rdr");
      shared_library lib(rdr_path);
      if (auto loader = lib.get<iPcbLoader * ()>("loader")) {
         if (auto pLoader = loader()) {
            auto str_fname = fname.string();
            shm::remove();
            shm::segment_name = fname.filename().replace_extension("smartdrc.data"/*to_string(boost::this_process::get_id())*/).string();
            shm::shared_directory = fname.parent_path().string();
            auto shared_mem_file = fss::path(shm::shared_directory) / shm::segment_name;
            if (__argc > 2 && !strcmp(__argv[2], "-connect")) {
               if (!fss::exists(shared_mem_file)) {
                  LOG("Connection failure: shared file does not exist");
                  exit(0);
               }
               shm::create();
            }
            else {
               LOG("Loading {0}", str_fname);
               auto shared_mem_file = fss::path(shm::shared_directory) / shm::segment_name;
               if (fss::exists(shared_mem_file)) {
                  fss::remove(shared_mem_file);
               }
               shm::create();
               pLoader->load(str_fname.c_str(), db);
               pLoader->release();
               LOG("Loading finished");
            }
         }
      }
   }
}
