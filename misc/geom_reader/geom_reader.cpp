// geom_reader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "pcb_loader.h"
#include "smartdrc.h"

cDatabase s_db;

int main(int argc, const char* argv[])
{
   try {
      load_design(std::filesystem::path(argv[1]), &s_db);
   }
   catch (...) {
      int i = 0;
   }
}
