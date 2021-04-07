#pragma once

#include "symbol_export.h"

#ifdef PCBDATABASE_EXPORTS
   #define PCBDATABASE_API SYMBOL_EXPORT
#else
   #define PCBDATABASE_API SYMBOL_IMPORT
#endif

#include "../pcb_database/pcb_db/generated/database.h"
