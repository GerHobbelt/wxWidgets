#pragma once

#include "interface.h"
#include "screen_coord_converter.h"
#include "render.h"

static inline wxPoint Round(const cCoordConverter::cScreenPoint::base &p)
{
   return {
      Round(p.m_x),
      Round(p.m_y)
   };
}
static inline wxRect Round(const cCoordConverter::cScreenRect::base &p)
{
   return {
      Round(p.m_left),
      Round(p.m_top),
      Round(p.width()),
      -Round(p.height())
   };
}

static inline cCoordConverter::cScreenPoint wxToScreen(const wxPoint &wx)
{
   return {
      (cCoordConverter::coord_t)wx.x,
      (cCoordConverter::coord_t)wx.y
   };
}

static inline cCoordConverter::cScreenRect wxToScreen(const wxRect &wx)
{
   return {
      (cCoordConverter::coord_t)wx.GetLeft(),
      (cCoordConverter::coord_t)wx.GetBottom(),
      (cCoordConverter::coord_t)wx.GetRight(),
      (cCoordConverter::coord_t)wx.GetTop()
   };
}

struct wxDib : public iBitmap
{
   uint32_t *m_data = nullptr;
   int m_width = 0, m_height = 0;

   wxDib()
   {
   }
   ~wxDib()
   {
      resize(0, 0);
   }
   wxDib(wxDib &&x)
      : m_data(std::exchange(x.m_data, nullptr))
      , m_width(std::exchange(x.m_width, 0))
      , m_height(std::exchange(x.m_height, 0))
   {
   }
   wxDib &operator=(wxDib &&x)
   {
      resize(0, 0);
      m_width = std::exchange(x.m_width, 0);
      m_height = std::exchange(x.m_height, 0);
      m_data = std::exchange(x.m_data, nullptr);
      return *this;
   }
   int width() const override
   {
      return m_width;
   }
   int height() const override
   {
      return m_height;
   }
   uint32_t *colors() override
   {
      return m_data;
   }

   void resize(int w, int h)
   {
      m_width = w;
      m_height = h;
      if (m_data) {
         delete[] m_data;
         m_data = nullptr;
      }
      if (auto size = w * h) {
         m_data = new uint32_t[size];
      }
   }
};
