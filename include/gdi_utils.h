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
