
#include "pch.h"

#include <future>
#include <sstream>

#include "MainFrm.h"
#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"

#include "options_imp.h"

using namespace std;
using namespace geom;

void CMFCUIView::DrawLayerBL2D(BLContext& context, cLayerDataBL2D* data)
{
   auto plane = data->plane;
   auto color = GetColor(data->color_id);

   context.setFillRule(BL_FILL_RULE_EVEN_ODD);
   BLRgba32 style(GetRValue(color), GetGValue(color), GetBValue(color));
   context.setFillStyle(style);
   context.setStrokeStyle(style);
   context.setStrokeCaps(BL_STROKE_CAP_ROUND);

   BLPath path;

   bool active_path = false;
   auto finish_path = [&]() {
      if (active_path) {
         context.fillPath(path);
         path.clear();
         active_path = false;
      }
   };

   data->visible = false;
   for (auto pshape = plane->shapes(data->viewport, data->object_type); pshape; ++pshape) {

      auto box = m_conv.WorldToScreen(pshape->rectangle());
      if (!box.height() && !box.width()) {
         continue;
      }

      data->visible = true;
      if (auto type = pshape->type(); type == iShape::Type::polyline) {
         if (!pshape->hole()) {
            finish_path();
            active_path = true;
         }
         cVertexIter iter = pshape->vertices();
         auto beg = m_conv.WorldToScreen(iter->beg());
         path.moveTo(beg.m_x, beg.m_y);
         for (; iter; ++iter) {
            auto& segment = *iter;
            auto end = m_conv.WorldToScreen(segment.end());
            if (iter.is_arc()) {
               auto circle = segment.center_and_radius();
               auto center = m_conv.WorldToScreen(circle.m_center);
               auto r = m_conv.WorldToScreen(abs(circle.m_radius));
               double start = -segment.start_angle(), sweep = -segment.sweep();
               path.arcTo(center.m_x, center.m_y, r, r, start, sweep);
            }
            else {
               path.lineTo(end.m_x, end.m_y);
            }
            beg = end;
         }
      }
      else {
         finish_path();
         switch (type) {
         case iShape::Type::circle:
            context.fillCircle(box.center().m_x, box.center().m_y, box.height() / 2);
            break;
         case iShape::Type::rectangle:
            context.fillRect(box.m_left, box.m_bottom, box.width(), box.height());
            break;
         case iShape::Type::segment:
         {
            cSegment seg = pshape->segment();
            auto beg = m_conv.WorldToScreen(seg.beg());
            auto end = m_conv.WorldToScreen(seg.end());
            if (Round(beg) != Round(end)) {
               auto width = 2 * m_conv.WorldToScreen(seg.width());
               context.setStrokeWidth(width);
               context.strokeLine(beg.m_x, beg.m_y, end.m_x, end.m_y);
            }
         }
         break;
         case iShape::Type::arc_segment:
         {
            cArc arc = pshape->arc_segment();
            auto beg = m_conv.WorldToScreen(arc.beg());
            auto end = m_conv.WorldToScreen(arc.end());
            if (Round(beg) != Round(end)) {
               auto circle = arc.center_and_radius();
               auto center = m_conv.WorldToScreen(circle.m_center);
               auto r = m_conv.WorldToScreen(abs(circle.m_radius));
               double start = -arc.start_angle(), sweep = -arc.sweep();
               auto width = 2 * m_conv.WorldToScreen(arc.width());
               context.setStrokeWidth(width);
               context.strokeArc(center.m_x, center.m_y, r, r, start, sweep);
            }
         }
         break;
         }
      }
   }
   finish_path();
}

void CMFCUIView::DrawBL2D(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter::cScreenRect& rect, iOptions* pOptions)
{
   BLImage blImage;
   auto width = pBitmap->width(), height = pBitmap->height();
   blImage.createFromData(width, height, BL_FORMAT_PRGB32, pBitmap->data(), width * sizeof(COLORREF));

   BLContextCreateInfo createInfo{};
   createInfo.threadCount = 16;

   geom::iEngine* ge = pDB->geom_engine();
   auto nTypes = (const int)geom::ObjectType::count;
   auto n_layers = (int)ge->planes() * nTypes;
   vector<cLayerDataBL2D> layer_info(n_layers);

   BLContext ctx(blImage, createInfo);

      for (int layer = n_layers - 1; layer >= 0; --layer) {
         auto& cur = layer_info[layer];
         cur.screen_rect = rect;
         int n_type = layer % nTypes;
         cur.object_type = geom::ObjectType(n_type);
         cur.viewport = m_conv.ScreenToWorld(rect);
         if (cur.plane = ge->plane(layer / nTypes)) {
            tie(cur.visible, cur.color_id) = pOptions->get_visibility(cur.plane->name(), GetObjectTypeName(cur.object_type));
            if (cur.visible) {
               DrawLayerBL2D(ctx, &cur);
            }
         }
      }
   }
