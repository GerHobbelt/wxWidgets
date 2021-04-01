
// spdlog_testView.cpp : implementation of the CspdlogtestView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "spdlog_test.h"
#endif

#include "spdlog_testDoc.h"
#include "spdlog_testView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CspdlogtestView

IMPLEMENT_DYNCREATE(CspdlogtestView, CView)

BEGIN_MESSAGE_MAP(CspdlogtestView, CView)
END_MESSAGE_MAP()

// CspdlogtestView construction/destruction

CspdlogtestView::CspdlogtestView() noexcept
{
	// TODO: add construction code here

}

CspdlogtestView::~CspdlogtestView()
{
}

BOOL CspdlogtestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CspdlogtestView drawing

void CspdlogtestView::OnDraw(CDC* /*pDC*/)
{
	CspdlogtestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CspdlogtestView diagnostics

#ifdef _DEBUG
void CspdlogtestView::AssertValid() const
{
	CView::AssertValid();
}

void CspdlogtestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CspdlogtestDoc* CspdlogtestView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CspdlogtestDoc)));
	return (CspdlogtestDoc*)m_pDocument;
}
#endif //_DEBUG


// CspdlogtestView message handlers
