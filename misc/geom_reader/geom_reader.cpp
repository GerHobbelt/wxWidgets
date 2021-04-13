// geom_reader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "pcb_loader.h"
#include "smartdrc.h"

std::unique_ptr<iDbHolder> s_db;

int main(int argc, const char* argv[])
{
   try {
      s_db.reset(load_design(std::filesystem::path(argv[1])));
   }
   catch (...) {
      int i = 0;
   }
}
