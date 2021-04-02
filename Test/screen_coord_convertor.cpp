#include "pch.h"

#include "screen_coord_converter.h"

class cScreenCoordConvertor : public ::testing::Test
{
public:
   cCoordConverter m_conv;
   const cCoordConverter::coord_t world_bound = 1'000'000.0;
   const cCoordConverter::coord_t screen_bound = 500.0;

   void SetUp() override
   {
      m_conv.SetWorld({-world_bound, -world_bound, world_bound, world_bound});
      m_conv.SetScreen({ -screen_bound, screen_bound, screen_bound, -screen_bound });
      m_conv.SetViewportCenter({ world_bound/2, world_bound/4 });
   }
};

using cWorldPoint = cCoordConverter::cWorldPoint;
using cWorldRect = cCoordConverter::cWorldRect;
using cScreenPoint = cCoordConverter::cScreenPoint;
using cScreenRect = cCoordConverter::cScreenRect;

TEST_F(cScreenCoordConvertor, WorldToScreen_Rect)
{
   auto dw = 10'000.0;
   cWorldRect wr({ world_bound / 2, world_bound / 4 + 1000 }, dw, 2 * dw);

   auto ds = 10.0;
   cScreenRect sr(cScreenPoint{ 0, -1 }, ds, 2 * ds);

   auto worlds = m_conv.WorldToScreen(wr);
   EXPECT_RECT_EQ(worlds, sr);
}
TEST_F(cScreenCoordConvertor, ScrollX)
{
   auto world = m_conv.World();
   auto screen = m_conv.Screen();
   auto scroll_range = m_conv.ScrollRange();

   m_conv.ScrollX(0);

   auto worlds = m_conv.WorldToScreen(world);
   EXPECT_DEQ(worlds.m_left, screen.m_right);

   auto sp = m_conv.ScrollPos();
   EXPECT_DEQ(sp.m_x, 0);

   m_conv.ScrollX(1);

   worlds = m_conv.WorldToScreen(world);
   EXPECT_DEQ(worlds.m_right, screen.m_left);

   sp = m_conv.ScrollPos();
   EXPECT_DEQ(sp.m_x, 1);

   m_conv.ScrollX(0.5);

   worlds = m_conv.WorldToScreen(world);
   auto wc = worlds.center();
   auto sc = screen.center();
   EXPECT_DEQ(wc.m_x, sc.m_x);

   sp = m_conv.ScrollPos();
   EXPECT_DEQ(sp.m_x, 0.5);
}
TEST_F(cScreenCoordConvertor, ScrollY)
{
   auto world = m_conv.World();
   auto screen = m_conv.Screen();
   auto scroll_range = m_conv.ScrollRange();

   m_conv.ScrollY(0);

   auto worlds = m_conv.WorldToScreen(world);
   EXPECT_DEQ(worlds.m_top, screen.m_bottom);

   auto sp = m_conv.ScrollPos();
   EXPECT_DEQ(sp.m_y, 0);

   m_conv.ScrollY(1);

   worlds = m_conv.WorldToScreen(world);
   EXPECT_DEQ(worlds.m_bottom, screen.m_top);

   sp = m_conv.ScrollPos();
   EXPECT_DEQ(sp.m_y, 1);

   m_conv.ScrollY(0.5);

   worlds = m_conv.WorldToScreen(world);
   auto wc = worlds.center();
   auto sc = screen.center();
   EXPECT_DEQ(wc.m_y, sc.m_y);

   sp = m_conv.ScrollPos();
   EXPECT_DEQ(sp.m_y, 0.5);
}
