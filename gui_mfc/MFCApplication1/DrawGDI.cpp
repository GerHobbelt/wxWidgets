
#include "pch.h"

#include <future>
#include <sstream>

#include "MainFrm.h"
#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"

#include "options_imp.h"

using namespace std;

void CMFCUIView::DrawLayerGDI(cLayerDataGDI* data)
{
   using namespace geom;

   auto& memDC = data->memDC;
   auto& plane = data->plane;
   auto& color_idx = data->color_id;

   int nSavedMemDC = SaveDC(memDC);

   auto color = GetColor(color_idx);
   cBrush br = CreateSolidBrush(color);
   SelectObject(memDC, br);
   cPen pen = CreatePen(PS_SOLID, 0, color);
   SelectObject(memDC, pen);

   struct line_desc
   {
      int lines = 0;

      enum { MAXLINES = 2000 };
      CPoint points[2 * MAXLINES];
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

   map<int, line_desc> lines;

   bool active_path = false;
   auto finish_path = [&]() {
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
   for (auto pshape = plane->shapes(data->viewport, data->object_type); pshape; ++pshape) {

      CRect box = Round(m_conv.WorldToScreen(pshape->rectangle()));
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
         CPoint beg = Round(m_conv.WorldToScreen(iter->beg()));
         MoveToEx(memDC, beg.x, beg.y, nullptr);
         for (; iter; ++iter) {
            auto& segment = *iter;
            CPoint end = Round(m_conv.WorldToScreen(segment.end()));
            if (iter.is_arc()) {
               SetArcDirection(memDC, segment.m_bulge < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
               auto world_rect = segment.center_and_radius().rectangle();
               CRect box = Round(m_conv.WorldToScreen(world_rect));
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
            CPoint beg = Round(m_conv.WorldToScreen(seg.beg()));
            CPoint end = Round(m_conv.WorldToScreen(seg.end()));
            if (beg != (POINT&)end) {
               int width = 2 * Round(m_conv.WorldToScreen(seg.width()));
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
            CPoint beg = Round(m_conv.WorldToScreen(seg.beg()));
            CPoint end = Round(m_conv.WorldToScreen(seg.end()));
            if (beg != (POINT&)end) {
               int width = 2 * Round(m_conv.WorldToScreen(seg.width()));
               cPen pen = CreatePen(PS_SOLID, width, color);
               auto old_pen = SelectObject(memDC, pen);
               SetArcDirection(memDC, seg.m_bulge < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
               auto world_rect = seg.center_and_radius().rectangle();
               CRect box = Round(m_conv.WorldToScreen(world_rect));
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

void CMFCUIView::DrawGDI(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter::cScreenRect& rect, iOptions* pOptions)
{
   geom::iEngine* ge = pDB->geom_engine();
   auto nTypes = (const int)geom::ObjectType::count;
   auto n_layers = (int)ge->planes() * nTypes;
   vector<cLayerDataGDI> layer_info(n_layers);

   cBrush brBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

   auto create_offscreen = [rect, &brBackground, pBitmap]() -> auto {
      cDC memDC = CreateCompatibleDC(pBitmap->dc());

      cBitmap hOffscreen = CreateCompatibleBitmap(pBitmap->dc(), Round(rect.width()), Round(rect.height()));
      SelectObject(memDC, hOffscreen);

      auto rc = Round(rect);
      FillRect(memDC, &rc, brBackground);

      return tuple(move(memDC), move(hOffscreen));
   };

   {
      vector<future<void>> futures(n_layers);
      for (int layer = n_layers - 1; layer >= 0; --layer) {
         auto& cur = layer_info[layer];
         cur.screen_rect = rect;
         int n_type = layer % nTypes;
         cur.object_type = geom::ObjectType(n_type);
         cur.viewport = m_conv.ScreenToWorld(rect);
         if (cur.plane = ge->plane(layer / nTypes)) {
            tie(cur.visible, cur.color_id) = pOptions->get_visibility(cur.plane->name(), GetObjectTypeName(cur.object_type));
            if (cur.visible) {
               forward_as_tuple(cur.memDC, cur.hOffscreen) = create_offscreen();
               //draw_layer(&cur);
               auto [x1, y1, x2, y2] = Round(rect);
               CRgn clip_rgn;
               clip_rgn.CreateRectRgn(x1, y1, x2, y2);
               SelectClipRgn(cur.memDC, clip_rgn);
               futures[layer] = async(&CMFCUIView::DrawLayerGDI, this, &cur);
            }
         }
      }
   }

   int width = Round(rect.width()), height = Round(rect.height());
   for (int layer = n_layers - 1; layer >= 0; --layer) {
      auto& cur = layer_info[layer];
      if (cur.visible) {
         SetBkColor(cur.memDC, RGB(0, 0, 0));
         cBitmap mask = CreateBitmap(width, height, 1, 1, NULL);
         cDC dcMask = CreateCompatibleDC(NULL);
         SelectObject(dcMask, mask);
         BitBlt(dcMask, 0, 0, width, height, cur.memDC, 0, 0, SRCCOPY);
         BitBlt(pBitmap->dc(), 0, 0, width, height, dcMask, 0, 0, SRCAND);
         BitBlt(pBitmap->dc(), 0, 0, width, height, cur.memDC, 0, 0, SRCPAINT);
      }
   }
}
