
#include "pch.h"

#include <future>
#include <sstream>

#include "MainFrm.h"
#include "MFCApplication1Doc.h"
#include "MFCApplication1View.h"

#include "options_imp.h"

using namespace std;
using namespace geom;

void CMFCUIView::DrawLayerBL2D(BLContext& context, cLayerDataBL2D* data) const
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
   auto viewport = data->conv.ScreenToWorld(data->conv.Screen());
   for (auto pshape = plane->shapes(viewport, data->object_type); pshape; ++pshape) {

      auto box = data->conv.WorldToScreen(pshape->rectangle());
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
         auto beg = data->conv.WorldToScreen(iter->beg());
         path.moveTo(beg.m_x, beg.m_y);
         for (; iter; ++iter) {
            auto& segment = *iter;
            auto end = data->conv.WorldToScreen(segment.end());
            if (iter.is_arc()) {
               auto circle = segment.center_and_radius();
               auto center = data->conv.WorldToScreen(circle.m_center);
               auto r = data->conv.WorldToScreen(abs(circle.m_radius));
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
               context.fillRect(box.m_left, box.m_top, box.width(), box.height());
               break;
            case iShape::Type::segment:
            {
               cSegment seg = pshape->segment();
               auto beg = data->conv.WorldToScreen(seg.beg());
               auto end = data->conv.WorldToScreen(seg.end());
               if (Round(beg) != Round(end)) {
                  auto width = 2 * data->conv.WorldToScreen(seg.width());
                  context.setStrokeWidth(width);
                  context.strokeLine(beg.m_x, beg.m_y, end.m_x, end.m_y);
               }
            }
            break;
            case iShape::Type::arc_segment:
            {
               cArc arc = pshape->arc_segment();
               auto beg = data->conv.WorldToScreen(arc.beg());
               auto end = data->conv.WorldToScreen(arc.end());
               if (Round(beg) != Round(end)) {
                  auto circle = arc.center_and_radius();
                  auto center = data->conv.WorldToScreen(circle.m_center);
                  auto r = data->conv.WorldToScreen(abs(circle.m_radius));
                  double start = -arc.start_angle(), sweep = -arc.sweep();
                  auto width = 2 * data->conv.WorldToScreen(arc.width());
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

void CMFCUIView::DrawBL2D(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter& conv, iOptions* pOptions) const
{
   BLImage blImage;
   auto width = pBitmap->width(), height = pBitmap->height();
   blImage.createFromData(width, height, BL_FORMAT_PRGB32, pBitmap->data(), width * sizeof(COLORREF));

   BLContextCreateInfo createInfo{};
   createInfo.threadCount = thread::hardware_concurrency();

   BLContext ctx(blImage, createInfo);

   geom::iEngine* ge = pDB->geom_engine();
   auto nTypes = (const int)geom::ObjectType::count;
   auto n_layers = (int)ge->planes() * nTypes;

      for (int layer = n_layers - 1; layer >= 0; --layer) {
      cLayerDataBL2D ld;
      ld.conv = conv;
      int n_type = layer % nTypes;
      ld.object_type = geom::ObjectType(n_type);
      if (ld.plane = ge->plane(layer / nTypes)) {
         tie(ld.visible, ld.color_id) = pOptions->get_visibility(ld.plane->name(), GetObjectTypeName(ld.object_type));
         if (ld.visible) {
            DrawLayerBL2D(ctx, &ld);
            }
         }
      }

   //BLImageCodec codec;
   //codec.findByName("BMP");
   //blImage.writeToFile("logs/render.bmp", codec);
   }
