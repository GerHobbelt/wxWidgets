#pragma once

#include <chrono>
#include <functional>
#include <spdlog/fmt/fmt.h>

#include "../core/core_dll.h"

namespace dbg {

   using log_stmt = std::function<std::string()>;
   using time_point = std::chrono::steady_clock::time_point;

   CORE_API void log(log_stmt log_f);

   CORE_API std::string thread_name();
} // namespace dbg

#define LOG(sfmt, ...) dbg::log([=] { return fmt::format(sfmt, __VA_ARGS__); });
