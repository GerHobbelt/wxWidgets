#pragma once

#ifdef _MSC_VER
  #define SYMBOL_EXPORT __declspec(dllexport)
  #define SYMBOL_IMPORT __declspec(dllimport)
#else
  #define SYMBOL_EXPORT
  #define SYMBOL_IMPORT
#endif

#ifdef CORE_EXPORTS
  #define CORE_API SYMBOL_EXPORT
#else
  #define CORE_API SYMBOL_IMPORT
#endif
