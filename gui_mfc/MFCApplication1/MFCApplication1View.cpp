
// MFCApplication1View.cpp : implementation of the CMFCUIView class
//

#include "pch.h"
#include "framework.h"

#include <future>
#include <sstream>

// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCApplication1.h"
#include "MainFrm.h"
#endif

#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"

#include "options_imp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// CMFCUIView

IMPLEMENT_DYNCREATE(CMFCUIView, CView)

BEGIN_MESSAGE_MAP(CMFCUIView, CView)
   ON_WM_CONTEXTMENU()
   ON_WM_RBUTTONUP()
   ON_WM_SIZE()
   ON_WM_MOUSEWHEEL()
   ON_WM_MOUSEMOVE()
   ON_COMMAND(ID_APP_ABOUT, &CMFCUIView::OnRestoreView)
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

namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

void CMFCUIView::OnInitialUpdate()
{
   OnRestoreView();
}

// CMFCUIView drawing

void CMFCUIView::OnDraw(CDC* pDC)
{
   DrawBL2D(pDC);
   //DrawGDI(pDC);
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

void CMFCUIView::OnSize(UINT nType, int cx, int cy)
{
   m_conv.SetScreenCenter({ cx / 2, cy / 2 });
}

BOOL CMFCUIView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
   ScreenToClient(&pt);
   m_conv.ZoomAround(pt, 1.5, zDelta > 0);
   Invalidate();
   return FALSE;
}

void CMFCUIView::OnMouseMove(UINT nFlags, CPoint pt)
{
   geom::cPoint wp = m_conv.ScreenToWorld(pt);
   CString msg;
   msg.Format("%g %g", wp.m_x, wp.m_y);
   if (auto pFrame = (CMainFrame*)GetParentFrame()) {
      pFrame->m_wndStatusBar.SetPaneText(0, msg);
   }
}

void CMFCUIView::OnRestoreView()
{
   using namespace geom;
   CMFCUIDoc* pDoc = GetDocument();
   iEngine* ge = pDoc->geom_engine();
   cRect bounds;// = pDoc->GetWorldRect();
   for (auto plane_id = ge->planes(); plane_id; --plane_id) {
      if (auto plane = ge->plane(plane_id - 1)) {
         bounds += plane->bounds();
      }
   }
   m_world_bounds = bounds;

   CRect rcClient;
   GetClientRect(&rcClient);
   m_conv.FitRect(bounds, rcClient);

   cOptionsImp opt(pDoc);
   auto view = opt.get_view();
   if (view.second) {
      m_conv.SetViewportCenter(view.first);
      m_conv.ZoomAround(rcClient.CenterPoint(), view.second / 2, true);
   }

   Invalidate();
}
