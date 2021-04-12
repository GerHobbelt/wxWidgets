
#include "pch.h"

#include "wx/dc.h"
#include "wx/dcclient.h"

#include "SmartDrcApp.h"
#include "SmartDrcFrame.h"
#include "BoardViewer.h"
#include "wx_utils.h"
#include "logger.h"

wxBEGIN_EVENT_TABLE(cDrawArea, cDrawAreaBase)
   EVT_SIZE(cDrawArea::OnSize)
   EVT_PAINT(cDrawArea::OnDraw)
   EVT_ERASE_BACKGROUND(cDrawArea::OnEraseBkgnd)
   EVT_MOTION(cDrawArea::OnMouseMove)
   EVT_MOUSEWHEEL(cDrawArea::OnMouseWheel)
wxEND_EVENT_TABLE()

cDrawArea::cDrawArea(wxWindow *parent, wxWindowID id, cPcbDesignDocument *doc)
   : cDrawAreaBase(parent, id)
   , m_document(doc)
{
   Init();
}

cDrawArea::~cDrawArea()
{
   Close();
}

void cDrawArea::Init()
{
   m_cvd.reset(new cOptionsImp(m_document->GetFilename()));
   OnRestoreView();
}

void cDrawArea::OnSize(wxSizeEvent& evt)
{
   evt.Skip();
   UpdateScrollBars(false);
   wxRect rcClient = GetClientRect();
   ++rcClient.height;
   ++rcClient.width;
   cScreenRect rc = wxToScreen(rcClient);
   m_conv.SetScreen(rc);
   Refresh();
}

void cDrawArea::OnEraseBkgnd(wxEraseEvent &evt)
{
}

void cDrawArea::OnDraw(wxPaintEvent& evt)
{
   using namespace std::chrono;
   auto time_start = steady_clock::now();

   wxRect rcDraw = GetClientRect();

   wxPaintDC dc(this);
   wxBitmap offbmp = Render(m_document->database(), rcDraw);
   dc.DrawBitmap(offbmp, rcDraw.GetLeftTop());

   auto time_finish = steady_clock::now();
   auto out_time = [this](const char *msg, auto time) {
      std::stringstream ss;
      ss << msg << duration_cast<milliseconds>(time).count();
      ss << "ms" << std::endl;
      if (auto frame = (cSmartDrcFrame*)wxGetApp().GetTopWindow()) {
         frame->SetStatusText(cSmartDrcFrame::Field_Elapsed, ss.str().c_str());
      }
   };
   out_time("Elapsed: ", time_finish - time_start);
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

   static const int nPage = 100;

   auto UpdateScrollBar(wxWindow* pWnd, wxOrientation orientation, const char* name, double page, double pos)
   {
      double d = 0;
      int nMax = 0, nPos = 0;
      if (page < 1 && !isnan(pos)) {

         //tex:
         // $$DoubleScrollPos(nPage) = page$$
         // $$nPage / (nMax - nPage + 1) = page$$
         // $$nMax = nPage / page + nPage - 1$$

         d = nPage / page + nPage - 1;
         nMax = Round(d) + 1;
         nPos = IntScrollPos(pos, d, nPage);
      }
      pWnd->SetScrollbar(orientation, nPos, nPage, nMax);

      //wxString msg;
      //msg.Format("%s scroll position: %d %g\n", name, si.nPos, pos);
      //OutputDebugString(msg);

      return d;
   };

   static const int SB_WHEEL = 100;

   auto ScrollPosition(wxWindow* pWnd, wxOrientation orientation, wxEventType evType, int nNewPos, geom::coord_t scroll_size)
   {
      int nPos = pWnd->GetScrollPos(orientation);
      int old_pos = nPos;

      if (evType == wxEVT_SCROLLWIN_TOP) {
         nPos = 0;
      }
      else if (evType == wxEVT_SCROLLWIN_BOTTOM) {
         nPos = INT_MAX;
      }
      else if (evType == wxEVT_SCROLLWIN_LINEUP) {
         --nPos;
      }
      else if (evType == wxEVT_SCROLLWIN_LINEDOWN) {
         ++nPos;
      }
      else if (evType == wxEVT_SCROLLWIN_PAGEUP) {
         nPos -= nPage;
      }
      else if (evType == wxEVT_SCROLLWIN_PAGEDOWN) {
         nPos += nPage;
      }
      else if (evType == wxEVT_SCROLLWIN_THUMBTRACK || evType == wxEVT_SCROLLWIN_THUMBRELEASE) {
         nPos = nNewPos;
      }
      else if (evType == SB_WHEEL) {
         nPos += nNewPos;
      }

      auto d = DoubleScrollPos(nPos, scroll_size, nPage);
      int pos = IntScrollPos(d, scroll_size, nPage);
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

void cDrawArea::UpdateScrollBars(bool bRedraw)
{
   auto scroll_page = m_conv.ScrollPage();
   auto scroll_pos = m_conv.ScrollPos();

   if (!isnan(scroll_pos.m_x) && !isnan(scroll_pos.m_y)) {
      LOG("  Position {0}:{1}", scroll_pos.m_x, scroll_pos.m_y);

      m_scroll_size.m_x = UpdateScrollBar(this, wxHORIZONTAL, "Horz", scroll_page.m_x, scroll_pos.m_x);
      m_scroll_size.m_y = UpdateScrollBar(this, wxVERTICAL, "Vert", scroll_page.m_y, scroll_pos.m_y);

      if (bRedraw) {
         Refresh();
      }
   }
}

void cDrawArea::OnMouseMove(wxMouseEvent& evt)
{
   auto wp = m_conv.ScreenToWorld(wxToScreen(evt.GetPosition()));
   auto s = fmt::format("{0:.5} {1:.5}", wp.m_x, wp.m_y);
   if (auto frame = (cSmartDrcFrame *)wxGetApp().GetTopWindow()) {
      frame->SetStatusText(cSmartDrcFrame::Field_Coord, s.c_str());
   }
}

void cDrawArea::OnMouseWheel(wxMouseEvent& evt)
{
   bool zoom_in = evt.GetWheelRotation() > 0;
   if (evt.ControlDown()) {
      wxPoint pt = evt.GetPosition();
      ScreenToClient(pt);
      LOG("Zooming {0} at {1}:{2}", zoom_in ? "in" : "out", pt.x, pt.y);
      m_conv.ZoomAround(wxToScreen(pt), zoom_in ? 2.0 / 3 : 1.5);
      UpdateScrollBars();
      return;
   }
   int delta = zoom_in ? -3 : 3;
   if (evt.ShiftDown()) {
      OnHScroll(SB_WHEEL, delta);
   }
   else {
      OnVScroll(SB_WHEEL, delta);
   }
}

wxBitmap cDrawArea::Render(cDatabase* pDB, const wxRect& rc) const
{
   if (m_cvd) {
      auto sz = rc.GetSize();
      if (int size = sz.x * sz.y) {
         LOG("    Rendering {0}:{1}:{2}:{3}", rc.x, rc.y, rc.width, rc.height);

         wxDib dib;
         dib.resize(sz.x, sz.y);

         auto pOpt = m_cvd.get();
         auto conv = m_conv.Rebind(wxToScreen(rc));

         DrawBL2D(pDB, &dib, conv, pOpt);

         auto size3 = 3 * size;
         auto image_colors = new uint8_t[size3];
         auto image_alpha = new uint8_t[size];
         auto colors = dib.colors();
         for (auto pic = image_colors, pia = image_alpha; pic < image_colors + size3;) {
            auto col = *colors++;
            *pia++ = col >> 24;
            *pic++ = col >> 16;
            *pic++ = col >> 8;
            *pic++ = col;
         }

         std::unique_ptr<uint8_t[]> image_color_data(image_colors), image_alpha_data(image_alpha);

         wxImage img;
         img.Create(sz, image_colors, image_alpha, true);

         LOG("    Rendering completed");

         return img;
      }
   }
   return wxBitmap();
}

void cDrawArea::OnRestoreView()
{
   using namespace geom;
   auto db = m_document->database();
   if (!db) {
      return;
   }

   cScreenRect rc(wxToScreen(GetClientRect()));
   m_conv.SetScreen(rc);

   m_cvd.reset(new cOptionsImp(m_document->GetFilename()));

   cRect bounds;
   for (auto&& layer: db->Layers()) {
      auto plane = layer.getPlane();
      bounds += plane->bounds();
   }

   m_conv.SetWorld(bounds);
   m_conv.FitRect(bounds);

   cOptionsImp opt(m_document->GetFilename());
   auto [center, zoom] = opt.get_view();
   if (zoom) {
      m_conv.SetViewportCenter(center);
      m_conv.ZoomAround(m_conv.Screen().center(), 2 / zoom);
   }

   UpdateScrollBars();
}

void cDrawArea::UpdateAfterScroll(const cScreenUpdateDesc screen_update_data)
{
   UpdateScrollBars(false);

   struct
   {
      wxBitmap offbmp;
      wxRect rc;

      void Render(cDrawArea *pDA, const cScreenRect &screen_rect)
      {
         rc = Round(screen_rect);
         if (auto pDoc = pDA->m_document) {
            offbmp = pDA->Render(pDoc->database(), rc);
         }
      }
   } rendered_data[std::size(screen_update_data.m_redraw_rect)];

   for (int i = 0; i < screen_update_data.m_redraw_rect_count; ++i) {
      rendered_data[i].Render(this, screen_update_data.m_redraw_rect[i]);
   }

   wxClientDC dc(this);
   wxRect rcSrc = Round(screen_update_data.m_copy_source);
   wxPoint ptDest = Round(screen_update_data.m_copy_dest);
   LOG("  Scrolling {0}:{1}:{2}:{3} to {4}:{5}", rcSrc.x, rcSrc.y, rcSrc.width, rcSrc.height, ptDest.x, ptDest.y);
   auto rc = dc.Blit(ptDest, rcSrc.GetSize(), &dc, rcSrc.GetLeftTop());

   for (int i = 0; i < screen_update_data.m_redraw_rect_count; ++i) {
      auto &dest = rendered_data[i].rc;
      if (auto &offbmp = rendered_data[i].offbmp; offbmp.IsOk()) {
         LOG("  Redrawing {0}:{1}:{2}:{3}", dest.x, dest.y, dest.width, dest.height);
         LOGX("     World {0}:{1}:{2}:{3}", auto wr = m_conv.ScreenToWorld(wxToScreen(dest)), wr.m_left, wr.m_top, wr.m_right, wr.m_bottom);
         dc.DrawBitmap(offbmp, dest.GetLeftTop());
      }
   }
}

void cDrawArea::OnHScroll(wxEventType evType, UINT nPos)
{
   if (auto d = ScrollPosition(this, wxHORIZONTAL, evType, nPos, m_scroll_size.m_x); d >= 0) {
      LOG("Scrolling horz to {0}", d);
      auto delta = m_conv.ScrollX(d);
      auto screen_update_data = m_conv.ScreenUpdateDataX(delta);
      UpdateAfterScroll(screen_update_data);
   }
}

void cDrawArea::OnVScroll(wxEventType evType, UINT nPos)
{
   if (auto d = ScrollPosition(this, wxVERTICAL, evType, nPos, m_scroll_size.m_y); d >= 0) {
      LOG("Scrolling vert to {0}", d);
      auto delta = m_conv.ScrollY(d);
      auto screen_update_data = m_conv.ScreenUpdateDataY(delta);
      UpdateAfterScroll(screen_update_data);
   }
}

bool cDrawArea::ProcessEvent(wxEvent& event)
{
   wxEventType evType = event.GetEventType();
   if (evType == wxEVT_SCROLLWIN_TOP ||
      evType == wxEVT_SCROLLWIN_BOTTOM ||
      evType == wxEVT_SCROLLWIN_LINEUP ||
      evType == wxEVT_SCROLLWIN_LINEDOWN ||
      evType == wxEVT_SCROLLWIN_PAGEUP ||
      evType == wxEVT_SCROLLWIN_PAGEDOWN ||
      evType == wxEVT_SCROLLWIN_THUMBTRACK ||
      evType == wxEVT_SCROLLWIN_THUMBRELEASE)
   {
      auto& ev = (wxScrollWinEvent&)event;
      auto ori = (wxOrientation)ev.GetOrientation();
      auto pos = ev.GetPosition();
      if (ori == wxHORIZONTAL) {
         OnHScroll(evType, pos);
      }
      else {
         OnVScroll(evType, pos);
      }
      return true;
   }
   return cDrawAreaBase::ProcessEvent(event);
}

wxIMPLEMENT_DYNAMIC_CLASS(cSmartDrcBoardView, wxView);

bool cSmartDrcBoardView::OnCreate(wxDocument *doc, long flags)
{
   if (!wxView::OnCreate(doc, flags)) {
      return false;
   }

   wxFrame *frame = wxGetApp().CreateChildFrame(this);
   wxASSERT(frame == GetFrame());

   m_draw_area.reset(new cDrawArea(frame, 1, (cPcbDesignDocument*)doc));

   frame->Show();

   return true;
}

void cSmartDrcBoardView::OnUpdate(wxView* sender, wxObject* hint)
{
   if (!sender) {
      // initial update?
      m_draw_area->Init();
   }
}
