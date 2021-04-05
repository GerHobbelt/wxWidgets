#pragma once

#include "../core/core_dll.h"

namespace core {
   CORE_API bool assert_failed(const char* file, int line, ...);
}

#define ASSERT(cond, ...) \
   if ( !(cond) && !core::assert_failed(__FILE__, __LINE__, __VA_ARGS__) ) {\
      assert(false);\
   }
