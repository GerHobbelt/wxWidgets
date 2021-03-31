#pragma once

#include "geom_model.h"

class cCoordConverter
{
   double m_a = 1000.0; // world units per pixel
   double m_bx = 0, m_by = 0; // world offset
   geom::cPoint m_screen_center;

public:
   struct cScreenPoint : public geom::cPoint
   {
      cScreenPoint()
      {
      }
      cScreenPoint(const geom::cPoint& x)
         : geom::cPoint(x)
      {
      }
      cScreenPoint(const CPoint& x)
         : geom::cPoint(geom::coord_t(x.x), geom::coord_t(x.y))
      {
      }
      cScreenPoint(int x, int y)
         : geom::cPoint(geom::coord_t(x), geom::coord_t(y))
      {
      }
      cScreenPoint(geom::coord_t x, geom::coord_t y)
         : geom::cPoint(x, y)
      {
      }
   };
   struct cScreenRect : public geom::cRect
   {
      cScreenRect()
      {
      }
      cScreenRect(const geom::cRect& x)
         : geom::cRect(x)
      {
      }
      cScreenRect(const CRect& x)
         : geom::cRect(geom::coord_t(x.left), geom::coord_t(x.top), geom::coord_t(x.right), geom::coord_t(x.bottom))
      {
      }
      cScreenRect(int left, int top, int right, int bottom)
         : geom::cRect(geom::coord_t(left), geom::coord_t(top), geom::coord_t(right), geom::coord_t(bottom))
      {
      }
      cScreenRect(geom::coord_t left, geom::coord_t top, geom::coord_t right, geom::coord_t bottom)
         : geom::cRect(left, top, right, bottom)
      {
      }
   };

   geom::coord_t WorldToScreenX(geom::coord_t world_coord) const noexcept
   {
      return (m_bx + world_coord) / m_a;
   }
   geom::coord_t WorldToScreenY(geom::coord_t world_coord) const noexcept
   {
      return (m_by - world_coord) / m_a;
   }
   geom::coord_t ScreenToWorldX(geom::coord_t screen_coord) const noexcept
   {
      return screen_coord * m_a - m_bx;
   }
   geom::coord_t ScreenToWorldY(geom::coord_t screen_coord) const noexcept
   {
      return m_by - screen_coord * m_a;
   }
   geom::coord_t WorldToScreen(geom::coord_t world_width) const noexcept
   {
      return world_width / m_a;
   }
   geom::cPoint WorldToScreen(const geom::cPoint& world_pt) const noexcept
   {
      auto x = WorldToScreenX(world_pt.m_x);
      auto y = WorldToScreenY(world_pt.m_y);
      return { x, y };
   }
   geom::cPoint ScreenToWorld(const cScreenPoint& screen_pt) const noexcept
   {
      auto x = ScreenToWorldX(screen_pt.m_x);
      auto y = ScreenToWorldY(screen_pt.m_y);
      return { x, y };
   }
   cScreenRect WorldToScreen(const geom::cRect& world_rect) const noexcept
   {
      auto top = WorldToScreenY(world_rect.m_top);
      auto left = WorldToScreenX(world_rect.m_left);
      auto right = WorldToScreenX(world_rect.m_right);
      auto bottom = WorldToScreenY(world_rect.m_bottom);
      return { left, top, right, bottom };
   }
   geom::cRect ScreenToWorld(const cScreenRect& screen_rect) const noexcept
   {
      auto top = ScreenToWorldY(screen_rect.m_top);
      auto left = ScreenToWorldX(screen_rect.m_left);
      auto right = ScreenToWorldX(screen_rect.m_right);
      auto bottom = ScreenToWorldY(screen_rect.m_bottom);
      return { left, top, right, bottom };
   }
   void SetScreenCenter(const cScreenPoint& new_center)
   {
      geom::cPoint old_center = std::exchange(m_screen_center, new_center);
      geom::cPoint delta = m_screen_center - old_center;
      m_bx += m_a * delta.m_x;
      m_by += m_a * delta.m_y;
   }
   void SetViewportCenter(const geom::cPoint& new_center)
   {
      m_bx = m_a * m_screen_center.m_x - new_center.m_x;
      m_by = m_a * m_screen_center.m_y + new_center.m_y;
   }
   void FitRect(const geom::cRect& world_rect, const cScreenRect& screen_rect)
   {
      m_a = max(world_rect.height() / screen_rect.height(), world_rect.width() / screen_rect.width());
      SetScreenCenter(screen_rect.center());
      SetViewportCenter(world_rect.center());
   }
   void ZoomAround(const cScreenPoint& pt, double k, bool zoom_in)
   {
      double k_actual = zoom_in ? 1 / k : k;
      m_bx += m_a * (k_actual - 1) * pt.m_x;
      m_by += m_a * (k_actual - 1) * pt.m_y;
      m_a *= k_actual;
   }
};

static inline int Round(double r)
{
   return int(r > 0 ? r + 0.5 : r - 0.5);
}
static inline CPoint Round(const cCoordConverter::cScreenPoint& p)
{
   return { Round(p.m_x), Round(p.m_y) };
}
static inline CRect Round(const cCoordConverter::cScreenRect& p)
{
   return { Round(p.m_left), Round(p.m_top), Round(p.m_right), Round(p.m_bottom) };
}
