
#include "pch.h"

#include "gdi_utils.h"
#include "screen_coord_converter.h"
#include "MFCApplication1Doc.h"

#include "options_imp.h"

using namespace std;

namespace {
   struct cLayerData
   {
      cDbTraits::eObjId object_type = cDbTraits::eObjId::Object;
      bool visible = false;
      COLORREF color = 0;
      cPlaneBase* plane = nullptr;
      cCoordConverter conv;
      int nSavedDC = 0;
      cBitmap hOffscreen;
      cDC memDC;
   };
}

void DrawLayerGDI(cLayerData* data)
{
   using namespace geom;

   auto& memDC = data->memDC;
   auto& plane = data->plane;

   int nSavedMemDC = SaveDC(memDC);

   auto color = data->color;
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
   auto viewport = data->conv.ScreenToWorld(data->conv.Screen());
   for (auto pshape = plane->shapes(viewport, (geom::ObjectType)data->object_type); pshape; ++pshape) {

      CRect box = Round(data->conv.WorldToScreen(pshape->rectangle()));
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
         CPoint beg = Round(data->conv.WorldToScreen(iter->beg()));
         MoveToEx(memDC, beg.x, beg.y, nullptr);
         for (; iter; ++iter) {
            auto& segment = *iter;
            CPoint end = Round(data->conv.WorldToScreen(segment.end()));
            if (iter.is_arc()) {
               SetArcDirection(memDC, segment.m_bulge < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
               auto world_rect = segment.center_and_radius().rectangle();
               CRect box = Round(data->conv.WorldToScreen(world_rect));
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
                  CPoint beg = Round(data->conv.WorldToScreen(seg.beg()));
                  CPoint end = Round(data->conv.WorldToScreen(seg.end()));
            if (beg != (POINT&)end) {
                     int width = 2 * Round(data->conv.WorldToScreen(seg.width()));
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
                  CPoint beg = Round(data->conv.WorldToScreen(seg.beg()));
                  CPoint end = Round(data->conv.WorldToScreen(seg.end()));
            if (beg != (POINT&)end) {
                     int width = 2 * Round(data->conv.WorldToScreen(seg.width()));
               cPen pen = CreatePen(PS_SOLID, width, color);
               auto old_pen = SelectObject(memDC, pen);
               SetArcDirection(memDC, seg.m_bulge < 0 ? AD_CLOCKWISE : AD_COUNTERCLOCKWISE);
               auto world_rect = seg.center_and_radius().rectangle();
                     CRect box = Round(data->conv.WorldToScreen(world_rect));
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

void DrawGDI(cDatabase* pDB, iBitmapGDI* pBitmap, const cCoordConverter conv, iOptions* pOptions)
{
   auto nTypes = 26;
   map<int, cPlaneBase *> planes;
   for (auto &&l: pDB->Layers()) {
      planes[-l.getLayerNumber()] = l.getPlane();
   }
   auto n_layers = (int)planes.size() * nTypes;
   vector<cLayerData> layer_info(n_layers);

   auto bk_color = pOptions->get_background_color();

   CRect rect = Round(conv.Screen());
   cBrush brBackground = CreateSolidBrush(bk_color);
   FillRect(pBitmap->dc(), &rect, brBackground);

   {
      vector<future<void>> futures(n_layers);
      for (auto &&[id, plane]: planes) {
         for (int object_type = 0; object_type < nTypes; ++object_type) {
            int layer = -id * object_type;
            auto& cur = layer_info[layer];
            cur.conv = conv;
            cur.object_type = cDbTraits::eObjId(object_type);
            if (cur.plane = plane) {
               auto plane_name = cur.plane->name();
               auto type_name = pDB->object_type_name(cur.object_type);
               tie(cur.visible, cur.color) = pOptions->get_visibility(plane_name, layer, type_name);
               if (cur.visible) {
                  cur.memDC = CreateCompatibleDC(pBitmap->dc());
                  cur.hOffscreen = CreateCompatibleBitmap(pBitmap->dc(), rect.Width(), rect.Height());
                  SelectObject(cur.memDC, cur.hOffscreen);
                  FillRect(cur.memDC, &rect, brBackground);
                  CRgn clip_rgn;
                  clip_rgn.CreateRectRgnIndirect(&rect);
                  SelectClipRgn(cur.memDC, clip_rgn);
                  futures[layer] = async(&DrawLayerGDI, &cur);
               }
            }
         }
      }
   }

   int width = rect.Width(), height = rect.Height();
   for (int layer = n_layers - 1; layer >= 0; --layer) {
      auto& cur = layer_info[layer];
      if (cur.visible) {
         TransparentBlt(pBitmap->dc(), 0, 0, width, height, cur.memDC, 0, 0, width, height, bk_color);
      }
   }
}
