
// MFCApplication1View.cpp : implementation of the CMFCUIView class
//

#include "pch.h"
#include "framework.h"
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

enum class eColor;

struct cColor
{
   COLORREF m_color;
   eColor m_id;

   cColor(eColor id, int r, int g, int b)
      : m_color(RGB(r, g, b))
      , m_id(id)
   {
   }
};

// color scheme from http://www.festra.com/cb/art-color.htm

enum class eColor
{
   Aqua,
   Black,
   Blue,
   Cream,
   Grey,
   Fuchsia,
   Green,
   Lime,
   Maroon,
   Navy,
   Olive,
   Purple,
   Red,
   Silver,
   Teal,
   White
};

cColor colors[] = {
   cColor(eColor::Aqua, 0, 255, 255),
   cColor(eColor::Black, 0, 0, 0),
   cColor(eColor::Blue, 0, 0, 255),
   cColor(eColor::Cream, 255, 251, 240),
   cColor(eColor::Grey, 128, 128, 128),
   cColor(eColor::Fuchsia, 255, 0, 255),
   cColor(eColor::Green, 0, 128, 0),
   cColor(eColor::Lime,	0, 255, 0),
   cColor(eColor::Maroon, 128, 0, 0),
   cColor(eColor::Navy, 0, 0, 128),
   cColor(eColor::Olive, 128, 128, 0),
   cColor(eColor::Purple, 255, 0, 255),
   cColor(eColor::Red, 255, 0, 0),
   cColor(eColor::Silver, 192, 192, 192),
   cColor(eColor::Teal, 0, 128, 128),
   cColor(eColor::White, 255, 255, 255)
};

struct cOptionsImp
{
   pt::ptree options;
   bool loaded = false;

   cOptionsImp(CMFCUIDoc* pDoc)
   {
      fs::path filename = (LPCTSTR)pDoc->GetPathName();
      filename.replace_extension(".prj");
      if (fs::exists(filename)) {
         pt::read_xml(filename.string(), options);
         loaded = true;
      }
   }
   pt::ptree::path_type layer_key(const char* layer, const char* objtype)
   {
      std::string retval = "options/drawing/";
      retval += layer;
      retval += "/";
      retval += objtype;
      retval += "/<xmlattr>";
      return pt::ptree::path_type(retval, '/');
   }
   std::pair<bool, eColor> get_visibility(const char* layer, const char* type)
   {
      if (loaded) {
         auto path = layer_key(layer, type);
         auto show = options.get<bool>(path / "visible", false);
         auto color = (eColor)options.get<int>(path / "color", 0);
         return { show, color };
      }
      return { true, eColor::Red };
   }
   pt::ptree::path_type view_key(const char* prop)
   {
      pt::ptree::path_type retval("options/viewport", '/');
      retval /= prop;
      retval /= "<xmlattr>/value";
      return retval;
   }
   std::pair<geom::cPoint, double> get_view()
   {
      try {
         pt::ptree::path_type path_scale = view_key("scale");
         pt::ptree::path_type path_x = view_key("center_x");
         pt::ptree::path_type path_y = view_key("center_y");
         geom::cPoint center(options.get<double>(path_x, 0), options.get<double>(path_y, 0));
         auto scale = options.get<double>(path_scale, 0);
         return { center, scale };
      }
      catch (...) {
         return {};
      }
   }
};

void CMFCUIView::OnInitialUpdate()
{
   OnRestoreView();
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

   CDC memDC;
   memDC.CreateCompatibleDC(pDC);
   int nSavedMemDC = memDC.SaveDC();

   HBITMAP hOffscreen = CreateCompatibleBitmap(pDC->GetSafeHdc(), rcClient.Width(), rcClient.Height());
   memDC.SelectObject(hOffscreen);

   CBrush brBackground;
   brBackground.Attach(GetStockObject(BLACK_BRUSH));
   memDC.FillRect(&rcClient, &brBackground);

   pDC->FillRect(&rcClient, &brBackground);

   cOptionsImp cvd(pDoc);

   {
      using namespace geom;
      cRect bounds = m_conv.ScreenToWorld(rcClient);
      iEngine* ge = pDoc->geom_engine();
      for (auto plane_id = ge->planes(); plane_id; --plane_id) {
         if (auto plane = ge->plane(plane_id - 1)) {

            auto [visible, color_idx] = cvd.get_visibility(plane->name(), "Pin");
            if (!visible) {
               continue;
            }

            auto color = colors[(int)color_idx].m_color;
            CBrush br(color);
            auto old_br = memDC.SelectObject(&br);
            CPen pen(PS_SOLID, 0, color);
            auto old_pen = memDC.SelectObject(&pen);

            bool active_path = false;
            auto finish_path = [&]() {
               if (active_path) {
                  memDC.EndPath();
                  memDC.FillPath();
                  active_path = false;
               }
            };

            for (auto pshape = plane->shapes(bounds, 0); pshape; ++pshape) {

               CRect box = m_conv.WorldToScreen(pshape->rectangle());
               if (!box.Height() && !box.Width()) {
                  continue;
               }

               if (auto type = pshape->type(); type == iShape::Type::polyline) {
                  if (!pshape->hole()) {
                     finish_path();
                     memDC.BeginPath();
                     active_path = true;
                  }
                  cVertexIter iter = pshape->vertices();
                  CPoint beg = m_conv.WorldToScreen(iter->beg());
                  MoveToEx(memDC, beg.x, beg.y, nullptr);
                  for (; iter; ++iter) {
                     auto& segment = *iter;
                     CPoint end = m_conv.WorldToScreen(segment.end());
                     if (iter.is_arc()) {
                        memDC.SetArcDirection(segment.m_radius < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
                        auto world_rect = segment.cCircle::rectangle();
                        CRect rect = m_conv.WorldToScreen(world_rect);
                        memDC.ArcTo(&rect, beg, end);
                     }
                     else {
                        memDC.LineTo(end);
                     }
                     beg = end;
                  }
               }
               else {
                  finish_path();
                  switch (type) {
                     case iShape::Type::circle:
                        memDC.Ellipse(box);
                        break;
                     case iShape::Type::rectangle:
                        memDC.Rectangle(&box);
                        break;
                     case iShape::Type::segment:
                        {
                           cSegment seg = pshape->segment();
                           CPoint beg = m_conv.WorldToScreen(seg.beg());
                           CPoint end = m_conv.WorldToScreen(seg.end());
                           if (beg != (POINT&)end) {
                              int width = 2 * m_conv.WorldToScreen(seg.width());
                              CPen pen(PS_SOLID, width, color);
                              auto old_pen = memDC.SelectObject(pen);
                              memDC.MoveTo(beg);
                              memDC.LineTo(end);
                              memDC.SelectObject(old_pen);
                           }
                        }
                        break;
                     case iShape::Type::arc_segment:
                        {
                           cArc seg = pshape->arc_segment();
                           CPoint beg = m_conv.WorldToScreen(seg.beg());
                           CPoint end = m_conv.WorldToScreen(seg.end());
                           if (beg != (POINT&)end) {
                              int width = 2 * m_conv.WorldToScreen(seg.width());
                              CPen pen(PS_SOLID, width, color);
                              auto old_pen = memDC.SelectObject(pen);
                              memDC.SetArcDirection(seg.m_radius < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
                              auto world_rect = seg.cCircle::rectangle();
                              CRect rect = m_conv.WorldToScreen(world_rect);
                              memDC.MoveTo(beg);
                              memDC.ArcTo(&rect, beg, end);
                              memDC.SelectObject(old_pen);
                           }
                        }
                        break;
                  }
               }
            }
            finish_path();
            memDC.SelectObject(old_pen);
            memDC.SelectObject(old_br);
         }
      }
   }

   pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

   memDC.RestoreDC(nSavedMemDC);
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

void CMFCUIView::OnSize(UINT nType, int cx, int cy)
{
   m_conv.SetScreenCenter({cx / 2, cy / 2});
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
