#include "pch.h"

TEST(TestCaseName, TestName) {
   EXPECT_EQ(1, 1);
   EXPECT_TRUE(true);
}

int main(int argc, char** argv)
{
   using namespace boost::dll;
   for (auto name : { "geometry.dll", "allegro.rdr", "dxf.rdr" }) {
      try {
         auto dll_path = program_location().parent_path() / name;
         auto lib = new shared_library(dll_path);
         //auto init = lib->get<void(int, char**)>("InitGoogleTest");
         //init(argc, argv);
      }
      catch (...) {
         //
      }
   }
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
