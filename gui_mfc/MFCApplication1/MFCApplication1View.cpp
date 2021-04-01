
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
   ON_WM_ERASEBKGND()
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
   m_cvd.reset(new cOptionsImp(GetDocument()));

   EnableScrollBarCtrl(SB_HORZ, true);
   EnableScrollBarCtrl(SB_VERT, true);

   OnRestoreView();
}

// CMFCUIView drawing

BOOL CMFCUIView::OnEraseBkgnd(CDC* pDC)
{
   return TRUE;
}

void CMFCUIView::OnDraw(CDC* pDC)
{
   CRect rcDraw;
   GetClientRect(&rcDraw);

   cDib offbmp = Render(pDC, rcDraw);
   BitBlt(*pDC, rcDraw.left, rcDraw.top, rcDraw.Width(), rcDraw.Height(), offbmp.dc(), 0, 0, SRCCOPY);
}

cDib CMFCUIView::Render(CDC* pDC, const CRect& rcDraw) const
{
   cDib retval;
   if (auto pDoc = GetDocument(); m_cvd && pDoc) {
      using namespace chrono;
      auto time_start = steady_clock::now();

      LOG("    Rendering {0}:{1}:{2}:{3}", rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom);

      retval.resize(rcDraw.Width(), rcDraw.Height(), *pDC);

      CRect rc = rcDraw;
      rc.OffsetRect(-rc.TopLeft());

      auto conv = m_conv;
      conv.SetScreen(rc);

      auto sc = cScreenRect(rcDraw).center();
      auto wc = m_conv.ScreenToWorld(sc);
      conv.SetViewportCenter(wc);

      auto stl = rcDraw.TopLeft();
      auto wtl = m_conv.ScreenToWorld(stl);
      auto stl1 = Round(conv.WorldToScreen(wtl));
      ASSERT(CPoint(0,0) == stl1);

      cBrush brBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
      FillRect(retval.dc(), &rc, brBackground);

      ASSERT(m_cvd);
      DrawBL2D(pDoc->database(), &retval, conv, m_cvd.get());
      //DrawGDI(pDoc->database(), &retval, conv, m_cvd.get());

      LOG("    Rendering completed");

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
   return retval;
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
   m_conv.SetScreen(CRect(0, 0, cx, cy));

   UpdateScrollBars(false);
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

   m_cvd.reset(new cOptionsImp(pDoc));

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
}

namespace {
   auto IntScrollPos(double nPos, double nMax, int nPage)
   {
      return Round(nPos * (nMax - nPage + 1));
   };
   auto DoubleScrollPos(int nPos, double nMax, int nPage)
   {
      return nPos / (nMax - nPage + 1);
   };

   auto UpdateScrollBar(CWnd* pWnd, int bar, const char* name, double page, double pos)
   {
      SCROLLINFO si{ sizeof SCROLLINFO };
      si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_RANGE | SIF_POS;
      si.nMin = 0;

      double d = 0;
      if (page < 1 && !isnan(pos)) {
         si.nPage = 100;

         //tex:
         // $$DoubleScrollPos(nPage) = page$$
         // $$nPage / (nMax - nPage + 1) = page$$
         // $$nMax = nPage / page + nPage - 1$$

         d = si.nPage / page + si.nPage - 1;
         si.nMax = Round(d);
         si.nPos = IntScrollPos(pos, d, si.nPage);
      }
      else {
         si.nMax = 0;
         si.nPage = 0;
         si.nPos = 0;
      }
      pWnd->SetScrollInfo(bar, &si, TRUE);

      //CString msg;
      //msg.Format("%s scroll position: %d %g\n", name, si.nPos, pos);
      //OutputDebugString(msg);

      return d;
   };

   static const int SB_WHEEL = 100;

   auto ScrollPosition(CWnd* pWnd, int nSBar, UINT nScrollCode, UINT nPos, CMFCUIView::coord_t scroll_size)
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

         case SB_WHEEL:
            si.nPos += nPos;
            break;
      }

      auto d = DoubleScrollPos(si.nPos, scroll_size, si.nPage);
      int pos = IntScrollPos(d, scroll_size, si.nPage);
      if (pos == old_pos) {
         return -1.0;
      }

      //CString msg;
      //if (auto pFrame = (CMainFrame*)pWnd->GetParentFrame()) {
      //   msg.Format("%d %d %g", si.nTrackPos, si.nPage, d);
      //   pFrame->m_wndStatusBar.SetPaneText(2, msg);
      //}

      return d;
   };
}

void CMFCUIView::UpdateScrollBars(bool bRedraw)
{
   auto scroll_page = m_conv.ScrollPage();
   auto scroll_pos = m_conv.ScrollPos();

   if (!isnan(scroll_pos.m_x) && !isnan(scroll_pos.m_y)) {
      LOG("  Position {0}:{1}", scroll_pos.m_x, scroll_pos.m_y);

      m_scroll_size.m_x = UpdateScrollBar(this, SB_HORZ, "Horz", scroll_page.m_x, scroll_pos.m_x);
      m_scroll_size.m_y = UpdateScrollBar(this, SB_VERT, "Vert", scroll_page.m_y, scroll_pos.m_y);

      if (bRedraw) {
         Invalidate();
      }
   }
}

BOOL CMFCUIView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
   if (nFlags & MK_CONTROL) {
      ScreenToClient(&pt);
      LOG("Zooming {0} at {1}:{2}", zDelta > 0 ? "in" : "out", pt.x, pt.y);
      m_conv.ZoomAround(pt, zDelta > 0 ? 2.0 / 3 : 1.5);
      UpdateScrollBars();
      return FALSE;
   }
   int delta = zDelta > 0 ? -3 : 3;
   if (nFlags & MK_SHIFT) {
      OnHScroll(SB_WHEEL, delta, nullptr);
   }
   else {
      OnVScroll(SB_WHEEL, delta, nullptr);
   }
   return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMFCUIView::UpdateAfterScroll(const cScreenUpdateDesc screen_update_data)
{
   UpdateScrollBars(false);

   CClientDC dc(this);
   struct {
      cDib offbmp;
      CRect rc;

      void Render(CMFCUIView* pView, CDC& dc, const cScreenRect& screen_rect)
      {
         rc = Round(screen_rect);
         offbmp = pView->Render(&dc, rc);
      }
   } rendered_data[size(screen_update_data.m_redraw_rect)];

   for (int i = 0; i < screen_update_data.m_redraw_rect_count; ++i) {
      rendered_data[i].Render(this, dc, screen_update_data.m_redraw_rect[i]);
   }

   CRect rcSrc = Round(screen_update_data.m_copy_source);
   CPoint ptDest = Round(screen_update_data.m_copy_dest);
   LOG("  Scrolling {0}:{1}:{2}:{3} to {4}:{5}", rcSrc.left, rcSrc.top, rcSrc.right, rcSrc.bottom, ptDest.x, ptDest.y);
   auto rc = BitBlt(dc, ptDest.x, ptDest.y, rcSrc.Width(), rcSrc.Height(), dc, rcSrc.TopLeft().x, rcSrc.TopLeft().y, SRCCOPY);

   for (int i = 0; i < screen_update_data.m_redraw_rect_count; ++i) {
      auto& dest = rendered_data[i].rc;
      auto offdc = rendered_data[i].offbmp.dc();
      LOG("  Redrawing {0}:{1}:{2}:{3}", dest.left, dest.top, dest.right, dest.bottom);
      auto wr = m_conv.ScreenToWorld(dest);
      LOG("     World {0}:{1}:{2}:{3}", wr.m_left, wr.m_top, wr.m_right, wr.m_bottom);
      BitBlt(dc, dest.left, dest.top, dest.Width(), dest.Height(), offdc, 0, 0, SRCCOPY);
   }
}

void CMFCUIView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar*)
{
   if (auto d = ScrollPosition(this, SB_HORZ, nSBCode, nPos, m_scroll_size.m_x); d >= 0) {
      LOG("Scrolling horz to {0}", d);
      auto [p0, p1] = m_conv.ScrollX(d);
      auto screen_update_data = m_conv.ScreenUpdateDataX(p0, p1);
      UpdateAfterScroll(screen_update_data);
   }
}

void CMFCUIView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar*)
{
   if (auto d = ScrollPosition(this, SB_VERT, nSBCode, nPos, m_scroll_size.m_y); d >= 0) {
      LOG("Scrolling vert to {0}", d);
      auto [p0, p1] = m_conv.ScrollY(d);
      auto screen_update_data = m_conv.ScreenUpdateDataY(p0, p1);
      UpdateAfterScroll(screen_update_data);
   }
}
