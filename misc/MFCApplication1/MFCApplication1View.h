
// MFCApplication1View.h : interface of the CMFCUIView class
//

#pragma once

#include "screen_coord_converter.h"
#include "options_imp.h"
#include "gdi_utils.h"
#include "options_imp.h"
#include "../smartdrc/DrawAreaBase.h"

class CMFCUIDoc;
class cDatabase;

interface iOptions;

class CMFCUIView
   : public CView
   , public cDrawAreaBase
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
   COLORREF GetColor(eColor idx) const;
   const char* GetObjectTypeName(geom::ObjectType type) const;
   cDib Render(cDatabase* pDB, CDC* pDC, const cScreenRect& rc) const;

   void UpdateScrollBars(bool bRedraw = true);
   void UpdateAfterScroll(const cScreenUpdateDesc screen_update_data);

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

