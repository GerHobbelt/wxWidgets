
#include "pch.h"

#include "DrawAreaBase.h"

void cDrawAreaBase::OnRestoreView(cDatabase* db, const cScreenRect& rc, const std::filesystem::path& project)
{
   if (!db) {
      return;
   }

   using namespace geom;

   m_conv.SetScreen(rc);

   m_cvd.reset(new cOptionsImp(project));

   cRect bounds;
   for (auto &&layer: db->Layers()) {
      auto plane = layer.getPlane();
      bounds += plane->bounds();
   }

   m_conv.SetWorld(bounds);
   m_conv.FitRect(bounds);

   //auto [center, zoom] = m_cvd->get_view();
   //if (zoom) {
   //   m_conv.SetViewportCenter(center);
   //   m_conv.ZoomAround(m_conv.Screen().center(), 2 / zoom);
   //}

   UpdateScrollBars();
}
