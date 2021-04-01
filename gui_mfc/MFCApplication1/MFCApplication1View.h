
// MFCApplication1View.h : interface of the CMFCUIView class
//

#pragma once

#include "screen_coord_converter.h"
#include "gdi_utils.h"
#include "options_imp.h"

class CMFCUIDoc;
class cDatabase;

interface iOptions;

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

   using eColor = cOptionsImp::eColor;

protected:
   cDib m_offscreen;
   cCoordConverter m_conv;

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

   struct cLayerData
   {
      geom::ObjectType object_type = (geom::ObjectType)0;
      bool visible = false;
      eColor color_id = (eColor)-1;
      geom::iPlane* plane = nullptr;
      geom::cRect viewport, screen_rect;
   };

   struct cLayerDataGDI
      : public cLayerData
   {
      int nSavedDC = 0;
      cBitmap hOffscreen;
      cDC memDC;
   };

   struct cLayerDataBL2D
      : public cLayerData
   {
   };

   void DrawLayerGDI(cLayerDataGDI* data);
   void DrawLayerBL2D(BLContext& ctx, cLayerDataBL2D* data);

   void DrawGDI(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter::cScreenRect& rect, iOptions* pOptions);
   void DrawBL2D(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter::cScreenRect& rect, iOptions* pOptions);

   COLORREF GetColor(eColor idx) const;
   const char* GetObjectTypeName(geom::ObjectType type) const;
   void UpdateScrollBars(bool bRedraw = true);

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
   afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
   afx_msg void OnRestoreView();
   afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
   afx_msg BOOL OnEraseBkgnd(CDC* pDC);
   DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MFCApplication1View.cpp
inline CMFCUIDoc* CMFCUIView::GetDocument() const
   { return reinterpret_cast<CMFCUIDoc*>(m_pDocument); }
#endif

