
// spdlog_testView.h : interface of the CspdlogtestView class
//

#pragma once


class CspdlogtestView : public CView
{
protected: // create from serialization only
	CspdlogtestView() noexcept;
	DECLARE_DYNCREATE(CspdlogtestView)

// Attributes
public:
	CspdlogtestDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CspdlogtestView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in spdlog_testView.cpp
inline CspdlogtestDoc* CspdlogtestView::GetDocument() const
   { return reinterpret_cast<CspdlogtestDoc*>(m_pDocument); }
#endif

