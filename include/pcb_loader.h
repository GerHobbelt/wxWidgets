#pragma once

#include "pcb_loader_callback.h"

interface iPcbLoader
{
   virtual bool load(const char* fname, iPcbLoaderCallback* db) = 0;
};
