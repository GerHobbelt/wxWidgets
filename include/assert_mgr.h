#pragma once

namespace core {
   bool assert_failed(const char* file, int line, ...)
   {
      return false;
   }
}

#define ASSERT(cond, ...) \
   if ( !(cond) && !core::assert_failed(__FILE__, __LINE__, __VA_ARGS__) ) {\
      assert(false);\
   }
