
// MFCApplication1View.cpp : implementation of the CMFCUIView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCApplication1.h"
#endif

#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCUIView

IMPLEMENT_DYNCREATE(CMFCUIView, CView)

BEGIN_MESSAGE_MAP(CMFCUIView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFCUIView construction/destruction

CMFCUIView::CMFCUIView() noexcept
{
	// TODO: add construction code here

}

CMFCUIView::~CMFCUIView()
{
}

BOOL CMFCUIView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CMFCUIView drawing

void CMFCUIView::OnDraw(CDC* pDC)
{
	CMFCUIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) {
		return;
	}

	int nSavedDC = pDC->SaveDC();

	CRect rcClient;
	GetClientRect(&rcClient);

	CBrush brBackground;
	brBackground.Attach(GetStockObject(BLACK_BRUSH));
	pDC->FillRect(&rcClient, &brBackground);

	CPoint org = pDoc->GetOrigin();
   pDC->SetViewportOrg(org.x, org.y);
	CSize ext = pDoc->GetExtents();
	pDC->SetViewportExt(ext.cx, ext.cy);

	geom::iPlane* plane = nullptr;
	geom::iEngine* ge = pDoc->geom_engine();
	int nLayers = 0, plane_id = 0;
	for (; ge->plane(&plane, nLayers); ++nLayers);
	for (int plane_id = nLayers - 1; plane_id > 0; --plane_id) {
		ge->plane(&plane, plane_id);
		geom::iShapeIter* shapes;
		if (plane->retrieve_shapes(&shapes, org.x, org.y, org.x + ext.cx, org.y + ext.cy, 0)) {
			for (geom::iShape* shape; shapes->next(&shape);) {
				switch (auto type = shape->type()) {
					case geom::iShape::Type::circle:
						break;
					case geom::iShape::Type::segment:
						break;
					case geom::iShape::Type::rectangle:
						break;
					case geom::iShape::Type::polyline:
						break;
				}
			}
			shapes->release();
		}
   }
  

	pDC->RestoreDC(nSavedDC);
}

void CMFCUIView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFCUIView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFCUIView diagnostics

#ifdef _DEBUG
void CMFCUIView::AssertValid() const
{
	CView::AssertValid();
}

void CMFCUIView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCUIDoc* CMFCUIView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCUIDoc)));
	return (CMFCUIDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFCUIView message handlers
