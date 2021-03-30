
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

// color scheme from http://www.festra.com/cb/art-color.htm

enum class CMFCUIView::eColor
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

CMFCUIView::cColor CMFCUIView::colors[] = {
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
   pair<bool, CMFCUIView::eColor> get_visibility(const char* layer, const char* type)
   {
      if (loaded) {
         auto path = layer_key(layer, type);
         auto show = options.get<bool>(path / "visible", false);
         auto color = (CMFCUIView::eColor)options.get<int>(path / "color", 0);
         return { show, color };
      }
      return { true, CMFCUIView::eColor::Red };
   }
   pt::ptree::path_type view_key(const char* prop)
   {
      pt::ptree::path_type retval("options/viewport", '/');
      retval /= prop;
      retval /= "<xmlattr>/value";
      return retval;
   }
   pair<geom::cPoint, double> get_view()
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

struct line_desc
{
   int lines = 0;

   static const int MAXLINES = 2000;
   CPoint points[2*MAXLINES];
   DWORD counts[MAXLINES];

   bool add(const CPoint beg, const CPoint end)
   {
      if (lines < MAXLINES) {
         counts[lines] = 2;
         points[2 * lines] = beg;
         points[2 * lines + 1] = end;
         ++lines;
         return true;
      }
      return false;
   }

   virtual void draw(HDC memDC, COLORREF color, int width)
   {
      if (lines) {
         cPen pen = CreatePen(PS_SOLID, width, color);
         auto old_pen = SelectObject(memDC, pen);
         PolyPolyline(memDC, points, counts, lines);
         SelectObject(memDC, old_pen);
         lines = 0;
      }
   }
};
struct line_desc1
{
   int num_points = 0;

   static const int POINTS = 2000;
   CPoint points[POINTS];

   bool add(HDC memDC, const CPoint beg, const CPoint end)
   {
      if (num_points < POINTS) {
         if (!num_points) {
            points[num_points++] = beg;
         }
         points[num_points++] = end;
         return true;
      }
      return false;
   }

   virtual void draw(HDC memDC, COLORREF color)
   {
      Polyline(memDC, points, num_points);
      num_points = 0;
   }
};

#define USE_POLYLINE

void CMFCUIView::DrawLayer(layer_data* data)
{
   using namespace geom;

   auto& memDC = data->memDC;
   auto& plane = data->plane;
   auto& color_id = data->color_id;

   int nSavedMemDC = SaveDC(memDC);

   auto color = colors[(int)color_id].m_color;
   cBrush br = CreateSolidBrush(color);
   SelectObject(memDC, br);
   cPen pen = CreatePen(PS_SOLID, 0, color);
   SelectObject(memDC, pen);

   map<int, line_desc> lines;
   line_desc1 path_lines;

   bool active_path = false;
   auto finish_path = [&]() {
      path_lines.draw(memDC, color);
      if (active_path) {
         EndPath(memDC);
         //CPoint pts[1000];
         //BYTE types[1000];
         //int c = GetPath(memDC, pts, types, 1000);
         FillPath(memDC);
         active_path = false;
      }
   };

   data->visible = false;
   for (auto pshape = plane->shapes(data->bounds, data->object_type); pshape; ++pshape) {

      CRect box = m_conv.WorldToScreen(pshape->rectangle());
      if (!box.Height() && !box.Width()) {
         continue;
      }

      data->visible = true;
      if (auto type = pshape->type(); type == iShape::Type::polyline) {
         if (!pshape->hole()) {
            finish_path();
            BeginPath(memDC);
            active_path = true;
         }
         cVertexIter iter = pshape->vertices();
         CPoint beg = m_conv.WorldToScreen(iter->beg());
         MoveToEx(memDC, beg.x, beg.y, nullptr);
         for (; iter; ++iter) {
            auto& segment = *iter;
            CPoint end = m_conv.WorldToScreen(segment.end());
            if (iter.is_arc()) {
               SetArcDirection(memDC, segment.m_radius < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
               auto world_rect = segment.cCircle::rectangle();
               CRect box = m_conv.WorldToScreen(world_rect);
               ArcTo(memDC, box.left, box.top, box.right, box.bottom, beg.x, beg.y, end.x, end.y);
            }
            else {
               LineTo(memDC, end.x, end.y);
            }
            beg = end;
         }
      }
      else {
         finish_path();
         switch (type) {
            case iShape::Type::circle:
               Ellipse(memDC, box.left, box.top, box.right, box.bottom);
               break;
            case iShape::Type::rectangle:
               Rectangle(memDC, box.left, box.top, box.right, box.bottom);
               break;
            case iShape::Type::segment:
               {
                  cSegment seg = pshape->segment();
                  CPoint beg = m_conv.WorldToScreen(seg.beg());
                  CPoint end = m_conv.WorldToScreen(seg.end());
                  if (beg != (POINT&)end) {
                     int width = 2 * m_conv.WorldToScreen(seg.width());
                     if (!lines[width].add(beg, end)) {
                        lines[width].draw(memDC, color, width);
                        bool rc = lines[width].add(beg, end);
                        ASSERT(rc);
                     }
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
                     cPen pen = CreatePen(PS_SOLID, width, color);
                     auto old_pen = SelectObject(memDC, pen);
                     SetArcDirection(memDC, seg.m_radius < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
                     auto world_rect = seg.cCircle::rectangle();
                     CRect box = m_conv.WorldToScreen(world_rect);
                     MoveToEx(memDC, beg.x, beg.y, nullptr);
                     ArcTo(memDC, box.left, box.top, box.right, box.bottom, beg.x, beg.y, end.x, end.y);
                     SelectObject(memDC, old_pen);
                  }
               }
               break;
         }
      }
   }
   finish_path();

   for (auto& [width, line] : lines) {
      line.draw(memDC, color, width);
   }

   RestoreDC(memDC, nSavedMemDC);
}

static const char* object_type_name[] = { // must correspond to geom::ObjectType
   "trace", "pin", "via", "areafill"
};

void CMFCUIView::OnDraw(CDC* pDC)
{
   CMFCUIDoc* pDoc = GetDocument();
   ASSERT_VALID(pDoc);
   if (!pDoc) {
      return;
   }

   using namespace chrono;
   auto time_start = steady_clock::now();

   cOptionsImp cvd(pDoc);

   CRect rcClient;
   GetClientRect(&rcClient);

   cBrush brBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   FillRect(*pDC, &rcClient, brBackground);

   auto create_offscreen = [rcClient, &brBackground, pDC]() -> auto {
      cDC memDC = CreateCompatibleDC(*pDC);

      cBitmap hOffscreen = CreateCompatibleBitmap(*pDC, rcClient.Width(), rcClient.Height());
      SelectObject(memDC, hOffscreen);

      FillRect(memDC , &rcClient, brBackground);

      return tuple(move(memDC), move(hOffscreen));
   };

   auto&& [offscreenDC, offscreenBmp] = create_offscreen();

   geom::iEngine* ge = pDoc->geom_engine();
   auto nTypes = (const int)geom::ObjectType::count;
   auto n_layers = (int)ge->planes() * nTypes;
   vector<layer_data> layer_info(n_layers);

   {
      vector<future<void>> futures(n_layers);
      for (int layer = n_layers - 1; layer >= 0; --layer) {
         auto& cur = layer_info[layer];
         int n_type = layer % nTypes;
         cur.object_type = geom::ObjectType(n_type);
         cur.bounds = m_conv.ScreenToWorld(rcClient);
         if (cur.plane = ge->plane(layer / nTypes)) {
            tie(cur.visible, cur.color_id) = cvd.get_visibility(cur.plane->name(), object_type_name[n_type]);
            if (cur.visible) {
               forward_as_tuple(cur.memDC, cur.hOffscreen) = create_offscreen();
               //draw_layer(&cur);
               futures[layer] = async(&CMFCUIView::DrawLayer, this, &cur);
            }
         }
      }
   }

   for (int layer = n_layers - 1; layer >= 0; --layer) {
      auto& cur = layer_info[layer];
      if (cur.visible) {
         SetBkColor(cur.memDC, RGB(0, 0, 0));
         cBitmap mask = CreateBitmap(rcClient.Width(), rcClient.Height(), 1, 1, NULL);
         cDC dcMask = CreateCompatibleDC(NULL);
         SelectObject(dcMask, mask);
         BitBlt(dcMask, 0, 0, rcClient.Width(), rcClient.Height(), cur.memDC, 0, 0, SRCCOPY);
         BitBlt(offscreenDC, 0, 0, rcClient.Width(), rcClient.Height(), dcMask, 0, 0, SRCAND);
         BitBlt(offscreenDC, 0, 0, rcClient.Width(), rcClient.Height(), cur.memDC, 0, 0, SRCPAINT);
      }
   }

   BitBlt(*pDC, 0, 0, rcClient.Width(), rcClient.Height(), offscreenDC, 0, 0, SRCCOPY);

   auto time_finish = steady_clock::now();
   auto out_time = [this](const char* msg, auto time) {
      stringstream ss;
      ss << msg << duration_cast<milliseconds>(time).count();
      ss << "ms" << endl;
      if (auto pFrame = (CMainFrame*)GetParentFrame()) {
         pFrame->m_wndStatusBar.SetPaneText(0, ss.str().c_str());
      }
   };
   out_time("Elapsed: ", time_finish - time_start);
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
