#pragma once

struct cDC
{
   cDC(HDC hDC = NULL)
      : m_hdc(hDC)
   {
      m_saved = SaveDC(hDC);
   }
   cDC(cDC&& x)
      : m_hdc(std::move(x.m_hdc))
      , m_saved(x.m_saved)
   {
   }
   ~cDC()
   {
      if (m_hdc) {
         HDC dc = m_hdc.release();
         RestoreDC(dc, m_saved);
         DeleteDC(dc);
      }
   }
   cDC& operator = (cDC&& x)
   {
      m_hdc = std::move(x.m_hdc);
      m_saved = x.m_saved;
      return *this;
   }
   operator HDC() const noexcept
   {
      return m_hdc.get();
   }

protected:
   int m_saved;
   std::unique_ptr<std::remove_pointer_t<HDC>> m_hdc;
};

template <typename H>
struct cGdiObj
{
   cGdiObj(H hobj = NULL)
      : m_hobj(hobj)
   {
   }
   cGdiObj(cGdiObj&& x)
      : m_hobj(std::move(x.m_hobj))
   {
   }
   ~cGdiObj()
   {
      if (m_hobj) {
         H hobj = m_hobj.release();
         DeleteObject(hobj);
      }
   }
   cGdiObj& operator = (cGdiObj&& x)
   {
      m_hobj = std::move(x.m_hobj);
      return *this;
   }
   operator H() const noexcept
   {
      return m_hobj.get();
   }

protected:
   std::unique_ptr<std::remove_pointer_t<H>> m_hobj;
};

using cBitmap = cGdiObj<HBITMAP>;
using cBrush = cGdiObj<HBRUSH>;
using cPen = cGdiObj<HPEN>;

interface iBitmap
{
   virtual int width() const = 0;
   virtual int height() const = 0;
   virtual int* data() const = 0;
   virtual HDC dc() const = 0;
};

struct cDib
   : public iBitmap
{
   BITMAPINFO m_bmi{ sizeof BITMAPINFO };
   void* m_data = nullptr;
   HBITMAP m_dib = 0;
   HDC m_dc = 0;
   int m_saved_dc = 0;

   cDib()
   {
   }
   ~cDib()
   {
      resize(0, 0, 0);
   }
   cDib(cDib&& x)
      : m_bmi(x.m_bmi)
      , m_data(std::exchange(x.m_data, nullptr))
      , m_dib(std::exchange(x.m_dib, HBITMAP(0)))
      , m_dc(std::exchange(x.m_dc, HDC(0)))
      , m_saved_dc(x.m_saved_dc)
   {
   }
   cDib& operator = (cDib&& x)
   {
      resize(0, 0, 0);

      m_bmi = x.m_bmi;
      m_data = std::exchange(x.m_data, nullptr);
      m_dib = std::exchange(x.m_dib, HBITMAP(0));
      m_dc = std::exchange(x.m_dc, HDC(0));
      m_saved_dc = x.m_saved_dc;

      return *this;
   }
   int width() const override
   {
      return m_bmi.bmiHeader.biWidth;
   }
   int height() const override
   {
      return -m_bmi.bmiHeader.biHeight;
   }
   int* data() const override
   {
      return (int*)m_data;
   }
   HDC dc() const override
   {
      return m_dc;
   }

   void resize(int w, int h, HDC dc)
   {
      if (m_bmi.bmiHeader.biWidth == w && m_bmi.bmiHeader.biHeight == -h) {
         return;
      }

      if (m_dib) {
         RestoreDC(m_dc, m_saved_dc);
         DeleteDC(m_dc);
         m_dc = 0;

         DeleteObject(m_dib);
         m_data = nullptr;
         m_dib = 0;
      }

      if (w && h && dc) {
         m_bmi.bmiHeader.biWidth = w;
         m_bmi.bmiHeader.biHeight = -h;
         m_bmi.bmiHeader.biBitCount = 32;
         m_bmi.bmiHeader.biPlanes = 1;
         m_dib = CreateDIBSection(nullptr, &m_bmi, DIB_RGB_COLORS, &m_data, NULL, 0);
         if (!m_dc) {
            m_dc = CreateCompatibleDC(dc);
            m_saved_dc = SaveDC(m_dc);
         }
         SelectObject(m_dc, m_dib);
      }
   }
};
