#pragma once
#pragma once

#include "interface.h"
#include "screen_coord_converter.h"
#include "render.h"

#include "qrect.h"
#include "qimage.h"

static inline cCoordConverter::cScreenRect csToScreen(const QRect &wx)
{
   return {(cCoordConverter::coord_t)wx.left(), (cCoordConverter::coord_t)wx.bottom(), (cCoordConverter::coord_t)wx.right(),
      (cCoordConverter::coord_t)wx.top()};
}

struct QDib : public iBitmap
{
   QImage m_image;

   QDib()
      : m_image(0, 0, QImage::Format_RGBA8888)
   {
   }
   ~QDib()
   {
      resize(0, 0);
   }
   QDib(QDib&&x)
      : m_image(std::move(x.m_image))
   {
   }
   QDib &operator=(QDib&&x)
   {
      resize(0, 0);
      m_image = std::move(x.m_image);
      return *this;
   }
   int width() const override
   {
      return m_image.width();
   }
   int height() const override
   {
      return m_image.height();
   }
   uint32_t *colors() override
   {
      return (uint32_t*)m_image.bits();
   }

   void resize(int w, int h)
   {
      QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
      std::swap(m_image, img);
   }
};
