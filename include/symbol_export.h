#pragma once

#ifdef _MSC_VER
  #define SYMBOL_EXPORT __declspec(dllexport)
  #define SYMBOL_IMPORT __declspec(dllimport)
#else
  #define SYMBOL_EXPORT
  #define SYMBOL_IMPORT
#endif
