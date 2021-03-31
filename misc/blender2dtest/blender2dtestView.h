
// blender2dtestView.h : interface of the CBlender2dTestView class
//

#pragma once

#include "screen_coord_converter.h"
#include "gdi_utils.h"

class CBlender2dTestView : public CView
{
protected: // create from serialization only
	CBlender2dTestView() noexcept;
	DECLARE_DYNCREATE(CBlender2dTestView)

// Attributes
public:
	CBlender2dTestDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
   BOOL PreCreateWindow(CREATESTRUCT& cs) override;
   void OnInitialUpdate() override;
	void OnDraw(CDC* pDC) override;  // overridden to draw this view
protected:

// Implementation
public:
	virtual ~CBlender2dTestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

   BLImage m_blImage;
   cCoordConverter m_conv;
   geom::cRect m_world_bounds;
};

#ifndef _DEBUG  // debug version in blender2dtestView.cpp
inline CBlender2dTestDoc* CBlender2dTestView::GetDocument() const
   { return reinterpret_cast<CBlender2dTestDoc*>(m_pDocument); }
#endif

