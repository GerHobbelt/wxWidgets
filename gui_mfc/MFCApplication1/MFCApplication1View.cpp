
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
   ON_WM_HSCROLL()
   ON_WM_VSCROLL()
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
   EnableScrollBarCtrl(SB_HORZ, true);
   EnableScrollBarCtrl(SB_VERT, true);

   OnRestoreView();
}

// CMFCUIView drawing

void CMFCUIView::OnDraw(CDC* pDC)
{
   auto pDoc = GetDocument();
   ASSERT_VALID(pDoc);
   if (!pDoc) {
      return;
   }

   using namespace chrono;
   auto time_start = steady_clock::now();

   cOptionsImp cvd(pDoc);

   CRect rcDraw;
   GetClipBox(*pDC, &rcDraw);

   cBrush brBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   FillRect(m_offscreen.dc(), &rcDraw, brBackground);

   cDib tmp;
   tmp.resize(rcDraw.Width(), rcDraw.Height(), *pDC);

   DrawBL2D(pDoc->database(), &tmp, rcDraw, &cvd);
   //DrawGDI(pDoc->database(), &tmp, rcDraw, &cvd);

   BitBlt(m_offscreen.dc(), rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(), tmp.dc(), 0, 0, SRCCOPY);
   BitBlt(*pDC, 0, 0, m_offscreen.width(), m_offscreen.height(), m_offscreen.dc(), 0, 0, SRCCOPY);

   auto time_finish = steady_clock::now();
   auto out_time = [this](const char* msg, auto time) {
      stringstream ss;
      ss << msg << duration_cast<milliseconds>(time).count();
      ss << "ms" << endl;
      if (auto pFrame = (CMainFrame*)GetParentFrame()) {
         pFrame->m_wndStatusBar.SetPaneText(1, CString(ss.str().c_str()));
      }
   };
   out_time("Elapsed: ", time_finish - time_start);
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
   CWindowDC dc(this);
   m_offscreen.resize(cx, cy, dc);

   m_conv.SetScreen(CRect(0, 0, cx, cy));

   UpdateScrollBars();
}

BOOL CMFCUIView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
   if (nFlags & MK_CONTROL) {
      ScreenToClient(&pt);
      m_conv.ZoomAround(pt, zDelta > 0 ? 2.0 / 3 : 1.5);
      UpdateScrollBars();
      Invalidate();
      return FALSE;
   }
   return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMFCUIView::OnMouseMove(UINT nFlags, CPoint pt)
{
   auto wp = m_conv.ScreenToWorld(pt);
   CString msg;
   msg.Format("%g %g", wp.m_x, wp.m_y);
   if (auto pFrame = (CMainFrame*)GetParentFrame()) {
      pFrame->m_wndStatusBar.SetPaneText(2, msg);
   }
}

void CMFCUIView::OnRestoreView()
{
   using namespace geom;
   CMFCUIDoc* pDoc = GetDocument();
   iEngine* ge = pDoc->geom_engine();

   cRect bounds;
   for (auto plane_id = ge->planes(); plane_id; --plane_id) {
      if (auto plane = ge->plane(plane_id - 1)) {
         bounds += plane->bounds();
      }
   }

   m_conv.SetWorld(bounds);
   m_conv.FitRect(bounds);

   cOptionsImp opt(pDoc);
   auto [center, zoom] = opt.get_view();
   if (zoom) {
      m_conv.SetViewportCenter(center);
      m_conv.ZoomAround(m_conv.Screen().center(), 2 / zoom);
   }

   UpdateScrollBars();
   Invalidate();
}

namespace {
   auto int_scroll_pos(double pos, int nMax, int nPage)
   {
      return Round(pos * (nMax - nPage + 1));
   };
   auto double_scroll_pos(int nPos, int nMax, int nPage)
   {
      return nPos / (double(nMax) - nPage + 1);
   };

   auto set_scroll_info(CWnd* pWnd, int bar, const char* name, double page, double pos)
   {
      SCROLLINFO si{ sizeof SCROLLINFO };
      si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_RANGE | SIF_POS;
      si.nMin = 0;

      if (page < 1) {
         si.nMax = 100;
         si.nPage = Round(page * si.nMax);
         si.nPos = int_scroll_pos(pos, si.nMax, si.nPage);
      }
      else {
         si.nMax = 0;
         si.nPage = 0;
         si.nPos = 0;
      }
      pWnd->SetScrollInfo(bar, &si, TRUE);

      CString msg;
      msg.Format("%s scroll position: %d %g\n", name, si.nPos, pos);
      OutputDebugString(msg);
   };

   auto ScrollPosition(CWnd* pWnd, int nSBar, UINT nScrollCode = -1, UINT nPos = -1)
   {
      SCROLLINFO si{ sizeof SCROLLINFO };
      si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
      pWnd->GetScrollInfo(nSBar, &si);

      int old_pos = si.nPos;

      switch (nScrollCode) {
         case SB_TOP:
            si.nPos = 0;
            break;
         case SB_BOTTOM:
            si.nPos = INT_MAX;
            break;
         case SB_LINEUP:
            --si.nPos;
            break;
         case SB_LINEDOWN:
            ++si.nPos;
            break;
         case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;
         case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;
         case SB_THUMBTRACK:
            si.nPos = nPos;
            break;
      }

      auto d = double_scroll_pos(si.nPos, si.nMax, si.nPage);
      int pos = int_scroll_pos(d, si.nMax, si.nPage);
      if (pos == old_pos) {
         return -1.0;
      }

      CString msg;
      if (auto pFrame = (CMainFrame*)pWnd->GetParentFrame()) {
         msg.Format("%d %d %g", si.nTrackPos, si.nPage, d);
         pFrame->m_wndStatusBar.SetPaneText(2, msg);
      }

      return d;
   };
}

void CMFCUIView::UpdateScrollBars()
{
   auto scroll_page = m_conv.ScrollPage();
   auto scroll_pos = m_conv.ScrollPos();

   set_scroll_info(this, SB_HORZ, "Horz", scroll_page.m_x, scroll_pos.m_x);
   set_scroll_info(this, SB_VERT, "Vert", scroll_page.m_y, scroll_pos.m_y);
}

void CMFCUIView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   auto d = ScrollPosition(this, SB_HORZ, nSBCode, nPos);
   if (d >= 0) {
      m_conv.ScrollX(d);
      UpdateScrollBars();
   }
   Invalidate();
}

void CMFCUIView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   auto d = ScrollPosition(this, SB_VERT, nSBCode, nPos);
   if (d >= 0) {
      m_conv.ScrollY(d);
      UpdateScrollBars();
   }
   Invalidate();
}
