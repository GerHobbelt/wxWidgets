#pragma once

#include "geom_model.h"

#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

class cCoordConverter
{
public:
   using coord_t = geom::coord_t;

   struct cScreenPoint : public geom::cPoint
   {
      cScreenPoint()
      {
      }
      cScreenPoint(const cScreenPoint& x)
         : geom::cPoint(x)
      {
      }
#ifdef _AFX
      cScreenPoint(const CPoint& x)
         : geom::cPoint(coord_t(x.x), coord_t(x.y))
      {
      }
      cScreenPoint(int x, int y)
         : geom::cPoint(coord_t(x), coord_t(y))
      {
      }
#endif
      cScreenPoint(coord_t x, coord_t y)
         : geom::cPoint(x, y)
      {
      }
   };
   struct cScreenRect : public geom::cRect
   {
      cScreenRect()
      {
      }
      cScreenRect(const cScreenRect& x)
         : geom::cRect(x)
      {
      }
      cScreenRect(const cScreenPoint& lb, const cScreenPoint& ut)
         : geom::cRect(lb, ut)
      {
      }
      cScreenRect(const cScreenPoint& c, coord_t cx, coord_t cy)
         : geom::cRect(c.m_x - cx / 2, c.m_y + cy / 2, c.m_x + cx / 2, c.m_y - cy / 2)
      {
      }
#ifdef _AFX
      cScreenRect(const CRect& x)
         : geom::cRect(coord_t(x.left), coord_t(x.bottom), coord_t(x.right), coord_t(x.top))
      {
      }
      cScreenRect(int left, int bottom, int right, int top)
         : geom::cRect(coord_t(left), coord_t(bottom), coord_t(right), coord_t(top))
      {
      }
#endif
      cScreenRect(coord_t left, coord_t bottom, coord_t right, coord_t top)
         : geom::cRect(left, bottom, right, top)
      {
      }

      cScreenPoint center() const noexcept
      {
         return (cScreenPoint &)geom::cRect::center();
      }
      coord_t height() const noexcept
      {
         return m_bottom - m_top;
      }
   };

   using cWorldPoint = geom::cPoint;
   using cWorldRect = geom::cRect;

protected:
   double a = 1000.0; // world units per pixel
   cScreenPoint b; // location of the world center on the screen, in screen coordinates

   cScreenRect m_screen;
   cWorldRect m_world;

public:
   cScreenRect Screen() const
   {
      return m_screen;
   }
   void SetScreen(const cScreenRect& screen)
   {
      auto viewport_center = ScreenToWorld(m_screen.center());
      m_screen = screen;
      SetViewportCenter(viewport_center);
   }
   cWorldRect World() const
   {
      return m_world;
   }
   void SetWorld(const cWorldRect& world)
   {
      m_world = world;
   }

   // conversion of distances
   coord_t WorldToScreen(coord_t world_width) const noexcept
   {
      return world_width / a;
   }
   coord_t ScreenToWorld(coord_t screen_width) const noexcept
   {
      return screen_width * a;
   }

   //tex:
   // WorldToScreen:
   //    $$\vec s = mirror_x(\vec w + \vec b) / a$$
   // ScreenToWorld:
   //    $$\vec w =  a \cdot mirror_x(\vec s) - \vec b$$

   // conversion of individual coordinates
   coord_t WorldToScreenX(coord_t world_coord) const noexcept
   {
      return (b.m_x + world_coord) / a;
   }
   coord_t WorldToScreenY(coord_t world_coord) const noexcept
   {
      return (b.m_y - world_coord) / a;
   }
   coord_t ScreenToWorldX(coord_t screen_coord) const noexcept
   {
      return screen_coord * a - b.m_x;
   }
   coord_t ScreenToWorldY(coord_t screen_coord) const noexcept
   {
      return b.m_y - screen_coord * a;
   }

   cScreenPoint WorldToScreen(const cWorldPoint& world_pt) const noexcept
   {
      auto x = WorldToScreenX(world_pt.m_x);
      auto y = WorldToScreenY(world_pt.m_y);
      return { x, y };
   }
   cWorldPoint ScreenToWorld(const cScreenPoint& screen_pt) const noexcept
   {
      auto x = ScreenToWorldX(screen_pt.m_x);
      auto y = ScreenToWorldY(screen_pt.m_y);
      return { x, y };
   }
   cScreenRect WorldToScreen(const cWorldRect& world_rect) const noexcept
   {
      auto top = WorldToScreenY(world_rect.m_top);
      auto left = WorldToScreenX(world_rect.m_left);
      auto right = WorldToScreenX(world_rect.m_right);
      auto bottom = WorldToScreenY(world_rect.m_bottom);
      return { left, bottom, right, top };
   }
   cWorldRect ScreenToWorld(const cScreenRect& screen_rect) const noexcept
   {
      auto top = ScreenToWorldY(screen_rect.m_top);
      auto left = ScreenToWorldX(screen_rect.m_left);
      auto right = ScreenToWorldX(screen_rect.m_right);
      auto bottom = ScreenToWorldY(screen_rect.m_bottom);
      return { left, bottom, right, top };
   }
   cScreenRect ScaleToScreen(const cWorldRect& world_rect) const noexcept
   {
      auto top = WorldToScreen(world_rect.m_top);
      auto left = WorldToScreen(world_rect.m_left);
      auto right = WorldToScreen(world_rect.m_right);
      auto bottom = WorldToScreen(world_rect.m_bottom);
      return { left, top, right, bottom };
   }
   cWorldRect ScaleToWorld(const cScreenRect& screen_rect) const noexcept
   {
      auto top = ScreenToWorld(screen_rect.m_top);
      auto left = ScreenToWorld(screen_rect.m_left);
      auto right = ScreenToWorld(screen_rect.m_right);
      auto bottom = ScreenToWorld(screen_rect.m_bottom);
      return { left, top, right, bottom };
   }

   // postcondition: new viewport center is mapped onto the screen center, zoom level unchanged
   void SetViewportCenter(const cWorldPoint& new_viewport_center)
   {
      //tex:
      //$$\vec m\_screen\_center = WorldToScreen_1(\vec viewport\_center)$$
      //$$\vec m\_screen\_center = (\vec b_1 + mirror\_x(\vec new\_viewport\_center)) / a$$
      //$$\vec b_1 = a \cdot \vec m\_screen\_center - mirror\_x(\vec new\_viewport\_center)$$

      b = (cScreenPoint&&)(m_screen.center() * a - new_viewport_center.mirror_x());
   }

   // postcondition: the center of world_rect is mapped onto the center of the screen,
   //   zoom level adjusted so that the world_rect is 'written in' into the screen
   void FitRect(const cWorldRect& world_rect)
   {
      a = std::max(world_rect.height() / m_screen.height(), world_rect.width() / m_screen.width());
      SetViewportCenter((cWorldPoint &&)world_rect.center());
   }

   // postcondition: a point that was mapped to pt remains mapped to pt, but the zoom is multiplied by k
   //    zoom out: k > 1
   //    zoom in: 1 > k > 0
   void ZoomAround(const cScreenPoint& pt, double k)
   {
      //tex:
      //$$\vec wpt = ScreenToWorld_0(\vec pt) = ScreenToWorld_1(\vec pt)$$
      //$$\vec pt \cdot a_0 - \vec b_0 = \vec pt \cdot a_0 \cdot k - \vec b_1$$
      //$$\vec b_1 = \vec b_0 + \vec pt \cdot a_0 \cdot (k - 1)$$
      //$$a_1 = \vec a_0 \cdot k$$

      b += pt * a * (k - 1);
      a *= k;
   }

   cScreenRect ScrollRange()
   {
      cScreenRect range = WorldToScreen(m_world);
      return { range.m_left - m_screen.width(), range.m_bottom, range.m_right, range.m_top - m_screen.height() };
   }
   cScreenPoint ScrollPos()
   {
      auto scroll_range = ScrollRange();
      auto dx = (m_screen.m_left - scroll_range.m_left) / scroll_range.width();
      auto dy = (m_screen.m_top - scroll_range.m_top) / scroll_range.height();
      return { std::clamp(dx, 0.0, 1.0), std::clamp(dy, 0.0, 1.0) };
   }
   cScreenPoint ScrollPage()
   {
      auto scroll_range = ScrollRange();
      auto dx = m_screen.width() / scroll_range.width();
      auto dy = m_screen.height() / scroll_range.height();
      return { std::clamp(dx, 0.0, 1.0), std::clamp(dy, 0.0, 1.0) };
   }
   // postcondition: ScreenToWorld(m_screen).m_right - WorldToScreen(m_world).m_left = x * ScrollRange.width()
   //    zoom is unchanged
   void ScrollX(double x)
   {
      //tex:
      // $$screen.right = WorldToScreen(world).left + x \cdot ScrollRange().width()$$
      // $$screen.right = WorldToScreen(world).left + x \cdot (WorldToScreen(world).width() + screen.width()$$
      // $$screen.right = (b.x + world.left)/a + x \cdot (world.width()/a + screen.width())$$
      // $$b.x = a \cdot (screen.right - x \cdot screen.width()) - (world.left + x \cdot world.width())$$

      b.m_x = a * (m_screen.m_right - x * m_screen.width()) - (m_world.m_left + x * m_world.width());
   }
   // postcondition: ScreenToWorld(m_screen).m_bottom - WorldToScreen(m_world).m_top = y * ScrollRange.height()
   //    zoom is unchanged
   void ScrollY(double y)
   {
      //tex:
      // $$screen.bottom = WorldToScreen(world).top + y \cdot ScrollRange().height()$$
      // $$screen.bottom = WorldToScreen(world).top + y \cdot (WorldToScreen(world).height() + screen.height()$$
      // $$screen.bottom = (b.y - world.top)/a + y \cdot (world.height()/a + screen.height())$$
      // $$b.y = a \cdot (screen.bottom - y .\cdot screen.height()) + (world.top - y \cdot world.height())$$

      auto screen_height = m_screen.height();
      auto world_height = m_world.height();
      b.m_y = a * (m_screen.m_bottom - y * screen_height) + (m_world.m_top - y * world_height);
   }
};

static inline int Round(double r)
{
   return int(r > 0 ? r + 0.5 : r - 0.5);
}

#ifdef _AFX
static inline CPoint Round(const cCoordConverter::cScreenPoint& p)
{
   return { Round(p.m_x), Round(p.m_y) };
}
static inline CRect Round(const cCoordConverter::cScreenRect& p)
{
   return { Round(p.m_left), Round(p.m_top), Round(p.m_right), Round(p.m_bottom) };
}
#endif

#pragma pop_macro("max")
#pragma pop_macro("min")
