
// MFCApplication1View.h : interface of the CMFCUIView class
//

#pragma once


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
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CMFCUIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in MFCApplication1View.cpp
inline CMFCUIDoc* CMFCUIView::GetDocument() const
   { return reinterpret_cast<CMFCUIDoc*>(m_pDocument); }
#endif

