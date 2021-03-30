
// MFCApplication1View.h : interface of the CMFCUIView class
//

#pragma once

class cCoordConverter
{
	double m_a = 1000.0; // world units per pixel
	double m_bx = 0, m_by = 0; // world offset
   CPoint m_screen_center;

public:
	static inline int Round(double r)
   {
      return int(r > 0 ? r + 0.5 : r - 0.5);
   }
   int WorldToScreenX(double world_coord) const noexcept
   {
      return Round((m_bx + world_coord) / m_a);
   }
   int WorldToScreenY(double world_coord) const noexcept
   {
      return Round((m_by - world_coord) / m_a);
   }
	double ScreenToWorldX(int screen_coord) const noexcept
   {
      return screen_coord * m_a - m_bx;
   }
   double ScreenToWorldY(int screen_coord) const noexcept
   {
      return m_by - screen_coord * m_a;
   }
   int WorldToScreen(double world_width) const noexcept
   {
      return Round(world_width / m_a);
   }
   CPoint WorldToScreen(const geom::cPoint& world_pt) const noexcept
	{
      auto x = WorldToScreenX(world_pt.m_x);
      auto y = WorldToScreenY(world_pt.m_y);
		return {x, y};
	}
   geom::cPoint ScreenToWorld(const CPoint& screen_pt) const noexcept
   {
      auto x = ScreenToWorldX(screen_pt.x);
      auto y = ScreenToWorldY(screen_pt.y);
      return {x, y};
   }
	CRect WorldToScreen(const geom::cRect& world_rect) const noexcept
   {
      auto top = WorldToScreenY(world_rect.m_top);
		auto left = WorldToScreenX(world_rect.m_left);
		auto right = WorldToScreenX(world_rect.m_right);
		auto bottom = WorldToScreenY(world_rect.m_bottom);
		return {left, top, right, bottom};
   }
   geom::cRect ScreenToWorld(const CRect& screen_rect) const noexcept
   {
      auto top = ScreenToWorldY(screen_rect.top);
      auto left = ScreenToWorldX(screen_rect.left);
      auto right = ScreenToWorldX(screen_rect.right);
      auto bottom = ScreenToWorldY(screen_rect.bottom);
      return {left, bottom, right, top};
   }
	void SetScreenCenter(const CPoint& new_center)
	{
      CPoint old_center = std::exchange(m_screen_center, new_center);
      CSize delta = m_screen_center - old_center;
      m_bx += m_a * delta.cx;
      m_by += m_a * delta.cy;
	}
   void SetViewportCenter(const geom::cPoint& new_center)
   {
      m_bx = m_a * m_screen_center.x - new_center.m_x;
      m_by = m_a * m_screen_center.y + new_center.m_y;
   }
   void FitRect(const geom::cRect& world_rect, const CRect& screen_rect)
   {
      m_a = max(world_rect.height() / screen_rect.Height(), world_rect.width() / screen_rect.Width());
      SetScreenCenter(screen_rect.CenterPoint());
      SetViewportCenter(world_rect.center());
   }
   void ZoomAround(const CPoint& pt, double k, bool zoom_in)
	{
		double k_actual = zoom_in ? 1 / k : k;
      m_bx += m_a * (k_actual - 1) * pt.x;
      m_by += m_a * (k_actual - 1) * pt.y;
      m_a *= k_actual;
	}
};

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

class CMFCUIView : public CView
{
protected: // create from serialization only
	CMFCUIView() noexcept;
	DECLARE_DYNCREATE(CMFCUIView)

// Attributes
public:
	CMFCUIDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	void OnDraw(CDC* pDC) override;  // overridden to draw this view
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	void OnInitialUpdate() override;
protected:

// Implementation
public:
	virtual ~CMFCUIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

   enum class eColor;

protected:
   struct cColor
   {
      COLORREF m_color;
      eColor m_id;

      cColor(eColor id, int r, int g, int b)
         : m_color(RGB(r, g, b))
         , m_id(id)
      {
      }
   };

   static cColor colors[];

   struct layer_data
   {
      geom::ObjectType object_type = (geom::ObjectType)0;
      bool visible = false;
      eColor color_id = (eColor)-1;
      geom::iPlane* plane = nullptr;
      geom::cRect bounds;
      int nSavedDC = 0;
      cBitmap hOffscreen;
      cDC memDC;
   };

   void DrawLayer(layer_data* data);

   cCoordConverter m_conv;
	geom::cRect m_world_bounds;

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
   afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
   afx_msg void OnRestoreView();
   DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MFCApplication1View.cpp
inline CMFCUIDoc* CMFCUIView::GetDocument() const
   { return reinterpret_cast<CMFCUIDoc*>(m_pDocument); }
#endif

