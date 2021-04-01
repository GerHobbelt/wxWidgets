
#include "pch.h"
#include "logger.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace std;

namespace dbg {

   //using log_entry = tuple<time_point, string, log_stmt>;
   //list<log_entry> log_list;

   auto logger = spdlog::basic_logger_mt("basic_logger", "logs/log.txt", true);

   CORE_API string thread_name()
   {
      return string();
   }

   CORE_API
   void log(log_stmt log_f)
   {
      //log_list.push_back({ chrono::steady_clock::now(), thread_name(), log_f });
      auto msg = log_f();
      logger->info(msg);
      logger->flush();
   }


   //auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
   //for (auto& [time, thread_name, l] : log_list) {
   //   auto s = l();
   //   logger->info(s);
   //}

} // namespace dbg
