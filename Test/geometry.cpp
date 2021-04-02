#include "pch.h"

#include "geom_model.h"

using namespace std;
using namespace geom;

class ArcTest : public ::testing::Test
{
public:

   void SetUp() override
   {
   }
};

struct cTestArc : public cArc
{
   using cArc::cArc;
};

TEST_F(ArcTest, BoundingBox)
{
   {
      cTestArc a({ 10, 0 }, { -10, 0 }, { 0, 0 }, 10);
      cRect r0{ -10, 0, 10, 10 }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
   {
      cTestArc a({ 0, 10 }, { 0, -10 }, { 0, 0 }, 10);
      cRect r0{ -10, -10, 0, 10 }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
   {
      cTestArc a({ 10, 0 }, { -10, 0 }, { 0, 0 }, -10);
      cRect r0{ -10, -10, 10, 0 }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
   {
      cTestArc a({ 0, 10 }, { 0, -10 }, { 0, 0 }, -10);
      cRect r0{ 0, -10, 10, 10 }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
   {
      double r = 10, a1 = pi() / 6, a2 = pi() / 4;
      cTestArc a({ r * cos(a1), r * sin(a1) }, { r * cos(a2), r * sin(a2) }, { 0, 0 }, r);
      cRect r0{ r * cos(a2), r * sin(a1), r * cos(a1), r * sin(a2) }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
   {
      double r = 10, a1 = pi() / 6, a2 = 3 * pi() / 4;
      cTestArc a({ r * cos(a1), r * sin(a1) }, { r * cos(a2), r * sin(a2) }, { 0, 0 }, r);
      cRect r0{ r * cos(a2), r * sin(a1), r * cos(a1), r }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
   {
      double r = 10, a1 = pi() / 6, a2 = 3 * pi() / 4;
      cTestArc a({ r * cos(a1), r * sin(a1) }, { r * cos(a2), r * sin(a2) }, { 0, 0 }, -r);
      cRect r0{ -r, -r, r, r * sin(a2) }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
   {
      double r = 10, a1 = pi() / 4, a2 = pi() / 6;
      cTestArc a({ r * cos(a1), r * sin(a1) }, { r * cos(a2), r * sin(a2) }, { 0, 0 }, r);
      cRect r0{ -r, -r, r, r }, r1 = a.rectangle();
      EXPECT_RECT_EQ(r0, r1);
   }
}
