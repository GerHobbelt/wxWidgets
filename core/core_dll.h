#pragma once
#include "symbol_export.h"

#ifdef CORE_EXPORTS
  #define CORE_API SYMBOL_EXPORT
#else
  #define CORE_API SYMBOL_IMPORT
#endif
