
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
      cDbTraits::eObjId object_type = cDbTraits::eObjId::Object;
      const char* type_name = nullptr;
      bool visible = false;
      uint32_t color = 0;
      cPlaneBase* plane = nullptr;
      cCoordConverter conv;
   };
}

#if 0
static size_t memsize = 0;
map<void*, size_t> s_sizes;
map<size_t, size_t> s_sizes_dist;

struct cAlloc : public iAllocator
{
   void* malloc(size_t size) override
   {
      memsize += size;
      auto retval = ::malloc(size);
      s_sizes[retval] = size;
      ++s_sizes_dist[size];
      return retval;
   }
   void free(void *p) override
   {
      size_t size = s_sizes[p];
      if (!size) {
         int i = 0;
      }
      memsize -= size;
      ::free(p);
   }
   cAlloc()
   {
      blSetAllocator(this);
   }
} s_alloc;
#endif

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
   auto viewport = data->conv.ScreenToWorld(data->conv.Screen());

   if (auto pshape = plane->shapes(viewport, (geom::ObjectType)data->object_type)) {

      context.setFillRule(BL_FILL_RULE_EVEN_ODD);
      cRgba32 style(data->color);
      context.setFillStyle(style);
      context.setStrokeStyle(style);
      context.setStrokeCaps(BL_STROKE_CAP_ROUND);

      BLPath path;

      bool filled = true;
      bool active_path = false;
      auto finish_path = [&]() {
         if (active_path) {
            if (filled) {
               context.fillPath(path);
            }
            else {
               context.strokePath(path);
            }
            path.clear();
            active_path = false;
         }
      };

      data->visible = false;

      for (; pshape; ++pshape) {

         auto box = data->conv.WorldToScreen(pshape->rectangle());
         if (box.height() < 0.5 && box.width() < 0.5) {
            continue;
         }

         data->visible = true;
         if (!pshape->hole()) {
            finish_path();
            active_path = true;
            filled = pshape->filled();
            if (!filled) {
               context.setStrokeWidth(1);
            }
         }

         switch (auto type = pshape->type()) {
            case iShape::Type::polyline:
               {
                  cVertexIter iter = pshape->vertices();
                  auto beg = data->conv.WorldToScreen(iter->beg());
                  path.moveTo(beg.m_x, beg.m_y);
                  for (; iter; ++iter) {
                     auto& segment = *iter;
                     auto end = data->conv.WorldToScreen(segment.end());
                     if (!iter.is_arc()) {
                        path.lineTo(end.m_x, end.m_y);
                     }
                     else {
                        auto circle = segment.center_and_radius();
                        auto center = data->conv.WorldToScreen(circle.m_center);
                        auto r = data->conv.WorldToScreen(abs(circle.m_radius));
                        double start = -segment.start_angle(), sweep = -segment.sweep();
                        path.arcTo(center.m_x, center.m_y, r, r, start, sweep);
                     }
                     beg = end;
                  }
               }
               break;
            case iShape::Type::circle:
               {
                  BLCircle cir(box.center().m_x, box.center().m_y, box.height() / 2);
                  path.addCircle(cir, pshape->hole());
               }
               break;
            case iShape::Type::rectangle:
               path.addRect(box.m_left, box.m_top, box.width(), box.height(), pshape->hole());
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
      finish_path();
   }
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

   map<int, tuple<int, int, cPlaneBase*>> planes;
   for (auto&& l : pDB->Layers()) {
      planes[-l.getLayerNumber()] = { l.getLayerType(), l.getMetalLayerNumber(), l.getPlane() };
   }
   for (const auto& [id, plane_desc]: planes) {
      auto [layer_type, metal_id, plane] = plane_desc;
      for (const auto& [object_type, desc]: plane->m_shape_types) {
         cLayerData ld;
         ld.conv = conv;
         ld.object_type = cDbTraits::eObjId(object_type);
         if (ld.plane = plane) {
            auto plane_name = id ? plane->name() : nullptr;
            ld.type_name = pDB->object_type_name(ld.object_type);
            int color_id = (layer_type == eLayerType::Conductive) ? metal_id : -1;
            tie(ld.visible, ld.color) = pOptions->get_visibility(plane_name, color_id, ld.type_name);
            if (ld.visible) {
               DrawLayerBL2D(ctx, &ld);
               ctx.flush(BL_CONTEXT_FLUSH_SYNC);
            }
         }
      }
   }
   //BLImageCodec codec;
   //codec.findByName("BMP");
   //blImage.writeToFile("logs/render.bmp", codec);
}
