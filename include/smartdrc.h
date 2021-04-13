#pragma once

#include <filesystem>

#include "symbol_export.h"
#include "database.h"

#ifdef SMARTDRC_EXPORTS
#define SMARTDRC_API SYMBOL_EXPORT
#else
#define SMARTDRC_API SYMBOL_IMPORT
#endif

interface iDbHolder
{
   virtual ~iDbHolder() {}
   virtual cDatabase* database() = 0;
   virtual void post_task(std::function<void()> task) = 0;
};

SMARTDRC_API
iDbHolder* load_design(const std::filesystem::path& fname);

SMARTDRC_API
void clear_design(cDatabase* db);
