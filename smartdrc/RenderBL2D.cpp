
#include "pch.h"

#include "database.h"
#include "options.h"
#include "render.h"

#include "screen_coord_converter.h"

#include "Blend2d.h"

using namespace std;
using namespace geom;

namespace {
   struct cLayerData
   {
      geom::ObjectType object_type = (geom::ObjectType)0;
      bool visible = false;
      uint32_t color = 0;
      geom::iPlane* plane = nullptr;
      cCoordConverter conv;
   };
}

inline bool rounded_eq(const cCoordConverter::cScreenPoint::base& p1, const cCoordConverter::cScreenPoint::base& p2)
{
   if (Round(p1.m_x) != Round(p2.m_x)) {
      return false;
   }
   if (Round(p1.m_y) != Round(p2.m_y)) {
      return false;
   }
   return true;
}

struct cRgba32 : public BLRgba32
{
   cRgba32(uint32_t x)
      : BLRgba32(((x & 0x00FF0000) >> 16) | ((x & 0x000000FF) << 16) | (x & 0x0000FF00) | 0xFF000000)
   {
   }
};

void DrawLayerBL2D(BLContext& context, cLayerData* data)
{
   auto plane = data->plane;

   context.setFillRule(BL_FILL_RULE_EVEN_ODD);
   cRgba32 style(data->color);
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
               if (!rounded_eq(beg, end)) {
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
               if (!rounded_eq(beg, end)) {
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

void DrawBL2D(cDatabase* pDB, iBitmap* pBitmap, const cCoordConverter conv, iOptions* pOptions)
{
   BLImage blImage;
   auto width = pBitmap->width(), height = pBitmap->height();
   blImage.createFromData(width, height, BL_FORMAT_PRGB32, pBitmap->colors(), width * sizeof(COLORREF));

   BLContextCreateInfo createInfo{};
   createInfo.threadCount = thread::hardware_concurrency();

   BLContext ctx(blImage, createInfo);
   auto color = pOptions->get_background_color();
   cRgba32 style(color);
   ctx.setFillStyle(style);
   ctx.fillRect(0, 0, width, height);

   geom::iEngine* ge = pDB->geom_engine();
   auto nTypes = (const int)geom::ObjectType::count;
   auto n_layers = (int)ge->planes() * nTypes;

   for (int layer = n_layers - 1; layer >= 0; --layer) {
      cLayerData ld;
      ld.conv = conv;
      int n_type = layer % nTypes;
      ld.object_type = geom::ObjectType(n_type);
      if (ld.plane = ge->plane(layer / nTypes)) {
         auto plane_name = ld.plane->name();
         auto type_name = pOptions->get_object_type_name(ld.object_type);
         tie(ld.visible, ld.color) = pOptions->get_visibility(plane_name, type_name);
         if (ld.visible) {
            DrawLayerBL2D(ctx, &ld);
         }
      }
   }

   //BLImageCodec codec;
   //codec.findByName("BMP");
   //blImage.writeToFile("logs/render.bmp", codec);
   }
