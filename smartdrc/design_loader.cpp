
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
namespace as = boost::asio;

static const char *database_name = "database";

struct cDbHolder
   : public iDbHolder
{
   shm::shared_memory* m_shared_mem = nullptr;

   as::io_service m_io_service;
   as::io_service::work* m_work;
   std::thread m_worker;

   cDbHolder()
      : m_work(new as::io_service::work(m_io_service))
      , m_worker([this] {m_io_service.run();})
   {
   }

   ~cDbHolder()
   {
      delete m_work;
      m_worker.join();
      shm::destroy(m_shared_mem);
   }
   cDatabase* database() override
   {
      auto db = m_shared_mem->find<cDatabase>(database_name).first;
      return db;
   }
   void post_task(std::function<void()> task) override
   {
      m_io_service.post(task);
   }
};

iDbHolder* load_design(const fss::path& fname)
{
   static map<path, path> s_loader_map{
      {".exb", "allegro"},
      {".xmlpcb", "xmlpcb"},
      {".dxf", "dxf"},
   };
   cDatabase* pDB = nullptr;
   unique_ptr<cDbHolder> retval;
   if (auto it = s_loader_map.find(fname.extension()); it != s_loader_map.end()) {
      shm::shared_directory = fname.parent_path().string();
      auto segment_name = fname.filename().replace_extension("smartdrc.data" /*to_string(boost::this_process::get_id())*/).string();
      auto shared_mem_file = fss::path(shm::shared_directory) / segment_name;

      bool connect = (__argc > 2 && !strcmp(__argv[2], "-connect"));
      if (connect) {
         if (!fss::exists(shared_mem_file)) {
            LOG("Connection failure: shared file does not exist");
            return nullptr;
         }
      }

      if (!connect && fss::exists(shared_mem_file)) {
         fss::remove(shared_mem_file);
      }

      retval.reset(new cDbHolder);
      retval->m_shared_mem = shm::create(segment_name);

      if (!connect) {
         auto rdr_path = program_location().parent_path() / (it->second.string() + ".rdr");
         shared_library lib(rdr_path);
         if (auto loader = lib.get<iPcbLoader *()>("loader")) {
            if (auto pLoader = loader()) {
               auto str_fname = fname.string();
               LOG("Loading {0}", str_fname);

               using namespace chrono;
               auto time_start = steady_clock::now();

               pDB = retval->m_shared_mem->construct<cDatabase>(database_name)();

               auto layer0_number = 0;
               auto layer0_name = "(All layers)";
               auto layer0 = pDB->createLayer();
               layer0->setName(layer0_name);
               auto plane0 = pDB->create<cPlaneBase>(layer0_number, layer0_name);
               layer0->setPlane(plane0);
               layer0->setLayerNumber(layer0_number);

               pLoader->load(str_fname.c_str(), pDB);

               static volatile auto objcount = cDbTraits::s_objcount;
               static volatile auto free_mem = retval->m_shared_mem->get_free_memory();

               pLoader->release();

               auto time_finish = steady_clock::now();
               LOG("Loading finished. {0} ms elapsed", duration_cast<milliseconds>(time_finish - time_start).count());
            }
         }
      }
   }
   return retval.release();
}
