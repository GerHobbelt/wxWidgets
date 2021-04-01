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

   using cWorldPoint = geom::cPoint;
   using cWorldRect = geom::cRect;

   struct cScreenTag
   {
   };

   struct cScreenPoint : public geom::tPoint<cScreenTag>
   {
      using base = geom::tPoint<cScreenTag>;
      using base::base;

      cScreenPoint(const base& x)
         : base(x)
      {
      }
#ifdef _AFX
      cScreenPoint(const CPoint& x)
         : base(coord_t(x.x), coord_t(x.y))
      {
      }
      cScreenPoint(int x, int y)
         : base(coord_t(x), coord_t(y))
      {
      }
#endif
   };
   struct cScreenRect : public geom::tRect<cScreenTag>
   {
      using base = geom::tRect<cScreenTag>;
      using base::base;

#ifdef _AFX
      cScreenRect(const CRect& x)
         : base(coord_t(x.left), coord_t(x.bottom), coord_t(x.right), coord_t(x.top))
      {
      }
      cScreenRect(int left, int bottom, int right, int top)
         : base(coord_t(left), coord_t(bottom), coord_t(right), coord_t(top))
      {
      }
#endif
      coord_t height() const noexcept
      {
         return m_bottom - m_top;
      }
   };

   struct cScreenUpdateDesc
   {
      bool m_copy = false;
      int m_redraw_rect_count = 0;
      cScreenRect m_copy_source;
      cScreenPoint m_copy_dest;
      cScreenRect m_redraw_rect[4];
   };

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
   //    $$\vec s = (mirror_x(\vec w) + \vec b) / a$$
   // ScreenToWorld:
   //    $$\vec w =  a \cdot mirror_x(\vec s - \vec b)$$

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

   // postcondition: new viewport center is mapped onto the screen center, zoom level unchanged
   void SetViewportCenter(const cWorldPoint& new_viewport_center)
   {
      //tex:
      //$$\vec m\_screen\_center = WorldToScreen_1(\vec viewport\_center)$$
      //$$\vec m\_screen\_center = (\vec b_1 + mirror\_x(\vec new\_viewport\_center)) / a$$
      //$$\vec b_1 = a \cdot \vec m\_screen\_center - mirror\_x(\vec new\_viewport\_center)$$

      b = m_screen.center() * a - (cScreenPoint&&)new_viewport_center.mirror_x();
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

   cScreenRect ScrollRange() const noexcept
   {
      cScreenRect range = WorldToScreen(m_world);
      return { range.m_left - m_screen.width(), range.m_bottom, range.m_right, range.m_top - m_screen.height() };
   }
   cScreenPoint ScrollPos() const noexcept
   {
      auto scroll_range = ScrollRange();
      auto dx = (m_screen.m_left - scroll_range.m_left) / scroll_range.width();
      auto dy = (m_screen.m_top - scroll_range.m_top) / scroll_range.height();
      return { std::clamp(dx, 0.0, 1.0), std::clamp(dy, 0.0, 1.0) };
   }
   cScreenPoint ScrollPage() const noexcept
   {
      auto scroll_range = ScrollRange();
      auto dx = m_screen.width() / scroll_range.width();
      auto dy = m_screen.height() / scroll_range.height();
      return { std::clamp(dx, 0.0, 1.0), std::clamp(dy, 0.0, 1.0) };
   }
   // postcondition: ScreenToWorld(m_screen).m_right - WorldToScreen(m_world).m_left = x * ScrollRange.width()
   //    zoom is unchanged
   auto ScrollX(double x)
   {
      auto p0 = ScrollPos();

      //tex:
      // $$screen.right = WorldToScreen(world).left + x \cdot ScrollRange().width()$$
      // $$screen.right = WorldToScreen(world).left + x \cdot (WorldToScreen(world).width() + screen.width()$$
      // $$screen.right = (b.x + world.left)/a + x \cdot (world.width()/a + screen.width())$$
      // $$b.x = a \cdot (screen.right - x \cdot screen.width()) - (world.left + x \cdot world.width())$$

      b.m_x = a * (m_screen.m_right - x * m_screen.width()) - (m_world.m_left + x * m_world.width());

      auto p1 = ScrollPos();
      return std::tuple{ p0, p1 };
   }
   // postcondition: ScreenToWorld(m_screen).m_bottom - WorldToScreen(m_world).m_top = y * ScrollRange.height()
   //    zoom is unchanged
   auto ScrollY(double y)
   {
      auto p0 = ScrollPos();

      //tex:
      // $$screen.bottom = WorldToScreen(world).top + y \cdot ScrollRange().height()$$
      // $$screen.bottom = WorldToScreen(world).top + y \cdot (WorldToScreen(world).height() + screen.height()$$
      // $$screen.bottom = (b.y - world.top)/a + y \cdot (world.height()/a + screen.height())$$
      // $$b.y = a \cdot (screen.bottom - y .\cdot screen.height()) + (world.top - y \cdot world.height())$$

      auto screen_height = m_screen.height();
      auto world_height = m_world.height();
      b.m_y = a * (m_screen.m_bottom - y * screen_height) + (m_world.m_top - y * world_height);

      auto p1 = ScrollPos();
      return std::tuple{ p0, p1 };
   }

   cScreenUpdateDesc ScreenUpdateDataX(const cScreenPoint& p0, const cScreenPoint& p1) const noexcept
   {
      cScreenUpdateDesc retval{ true, 1, m_screen, m_screen.top_left(), {m_screen} };

      auto scroll_range = ScrollRange();
      double delta = std::abs(p1.m_x - p0.m_x) * scroll_range.width();
      if (p1.m_x > p0.m_x) {
         retval.m_copy_source.m_left += delta;
         auto& r = retval.m_redraw_rect[0];
         r.m_left = r.m_right - delta;
      }
      else {
         retval.m_copy_source.m_right -= delta;
         retval.m_copy_dest.m_x += delta;
         retval.m_redraw_rect->m_right = retval.m_redraw_rect->m_left + delta;
      }
      return retval;
   }

   cScreenUpdateDesc ScreenUpdateDataY(const cScreenPoint& p0, const cScreenPoint& p1) const noexcept
   {
      cScreenUpdateDesc retval{ true, 1, m_screen, m_screen.top_left(), {m_screen} };

      auto scroll_range = ScrollRange();
      double delta = std::abs(p1.m_y - p0.m_y) * scroll_range.height();
      if (p1.m_y > p0.m_y) {
         retval.m_copy_source.m_top += delta;
         auto& r = retval.m_redraw_rect[0];
         r.m_top = r.m_bottom - delta;
      }
      else {
         retval.m_copy_source.m_bottom -= delta;
         retval.m_copy_dest.m_y += delta;
         retval.m_redraw_rect->m_bottom = retval.m_redraw_rect->m_top + delta;
      }
      return retval;
   }
};

static inline int Round(double r)
{
   return int(r > 0 ? r + 0.5 : r - 0.5);
}

#ifdef _AFX
static inline CPoint Round(const cCoordConverter::cScreenPoint::base& p)
{
   return { Round(p.m_x), Round(p.m_y) };
}
static inline CRect Round(const cCoordConverter::cScreenRect::base& p)
{
   return { Round(p.m_left), Round(p.m_top), Round(p.m_right), Round(p.m_bottom) };
}
#endif

#pragma pop_macro("max")
#pragma pop_macro("min")
