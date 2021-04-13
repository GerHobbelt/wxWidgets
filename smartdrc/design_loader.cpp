
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
   //TBD: clear segment
}

namespace fss = filesystem;

cDatabase* load_design(const fss::path& fname)
{
   static map<path, path> s_loader_map{
      {".exb", "allegro"},
      {".xmlpcb", "xmlpcb"},
      {".dxf", "dxf"},
   };
   static int db_index = 0;
   static const char* database_name = "database";
   cDatabase* pDB = nullptr;
   if (auto it = s_loader_map.find(fname.extension()); it != s_loader_map.end()) {
      if (!shm::exists()) {
         shm::segment_name = fname.filename().replace_extension("smartdrc.data" /*to_string(boost::this_process::get_id())*/).string();
         shm::shared_directory = fname.parent_path().string();
      }

      bool connect = (__argc > 2 && !strcmp(__argv[2], "-connect"));
      auto shared_mem_file = fss::path(shm::shared_directory) / shm::segment_name;
      if (connect) {
         if (!fss::exists(shared_mem_file)) {
            LOG("Connection failure: shared file does not exist");
            exit(0);
         }
      }

      if (!shm::exists()) {
         if (!connect && fss::exists(shared_mem_file)) {
            fss::remove(shared_mem_file);
         }
         shm::create();
      }

      if (!connect) {
         auto rdr_path = program_location().parent_path() / (it->second.string() + ".rdr");
         shared_library lib(rdr_path);
         if (auto loader = lib.get<iPcbLoader *()>("loader")) {
            if (auto pLoader = loader()) {
               auto str_fname = fname.string();
               LOG("Loading {0}", str_fname);

               using namespace chrono;
               auto time_start = steady_clock::now();

               pDB = shm::mshm.construct<cDatabase>(fmt::format("{}{}", database_name, db_index++).c_str())();

               auto layer0_number = 0;
               auto layer0_name = "(All layers)";
               auto layer0 = pDB->createLayer();
               layer0->setName(layer0_name);
               auto plane0 = pDB->create<cPlaneBase>(layer0_number, layer0_name);
               layer0->setPlane(plane0);
               layer0->setLayerNumber(layer0_number);

               pLoader->load(str_fname.c_str(), pDB);

               static volatile auto objcount = cDbTraits::s_objcount;
               static volatile auto free_mem = shm::mshm.get_free_memory();

               pLoader->release();

               auto time_finish = steady_clock::now();
               LOG("Loading finished. {0} ms elapsed", duration_cast<milliseconds>(time_finish - time_start).count());
            }
         }
      }
   }
   return pDB;
}
