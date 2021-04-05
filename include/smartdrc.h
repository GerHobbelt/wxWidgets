#pragma once
#include "symbol_export.h"
#include "database.h"

#ifdef SMARTDRC_EXPORTS
#define SMARTDRC_API SYMBOL_EXPORT
#else
#define SMARTDRC_API SYMBOL_IMPORT
#endif

SMARTDRC_API
void load_design(const std::filesystem::path& fname, cDatabase* db);

SMARTDRC_API
void clear_design(cDatabase* db);
