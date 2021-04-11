#pragma once

#include <chrono>
#include <functional>
#include <spdlog/fmt/fmt.h>

#include "../core/core_dll.h"

namespace dbg {

   using log_stmt = std::function<std::string()>;
   using time_point = std::chrono::steady_clock::time_point;

   CORE_API bool logging_enabled();
   CORE_API void enable_logging(bool = true);
   CORE_API void log(log_stmt log_f);

   CORE_API std::string thread_name();
} // namespace dbg

#define LOG(sfmt, ...) if (dbg::logging_enabled()) dbg::log([=] { return fmt::format(sfmt, __VA_ARGS__); });
#define LOGX(sfmt, stmt, ...) if (dbg::logging_enabled()) { stmt; dbg::log([=] { return fmt::format(sfmt, __VA_ARGS__); });}
