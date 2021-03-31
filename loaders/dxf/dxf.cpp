
#include "pch.h"
#include "pcb_loader.h"
#include "geom_reader_base.h"

#include "dl_dxf.h"
#include "dl_creationadapter.h"

class cDxfLoader
   : public iPcbLoader
   , public DL_CreationAdapter
   , public cGeomReaderBase
{
   static inline double deg2rad(double angle)
   {
      return angle / 180 * geom::pi();
   }

   bool load(const char* fname, iPcbLoaderCallback* db) override
   {
      m_ge = db->geom_engine();

      DL_Dxf dxf;
      if (!dxf.in(fname, this)) { // if file open failed
         return false;
      }

      if (m_current_shape) {
         m_current_shape->commit();
      }

      return true;
   }

   void addLayer(const DL_LayerData& data) override
   {
      m_current_plane = m_ge->create_plane(0, data.name.c_str());
   }
   void addPoint(const DL_PointData& data) override
   {
   }
   void addLine(const DL_LineData& data) override
   {
      geom::iShape* ps = nullptr;
      m_ge->create_segment(&ps, data.x1, data.y1, data.x2, data.y2);
      add_to_plane(ps, m_current_plane->name(), geom::ObjectType::trace);
   }
   void addArc(const DL_ArcData& data) override
   {
      geom::iShape* ps = nullptr;
      auto a1 = deg2rad(data.angle1);
      auto a2 = deg2rad(data.angle2);
      geom::coord_t x1 = data.cx + data.radius * cos(a1);
      geom::coord_t y1 = data.cy + data.radius * sin(a1);
      geom::coord_t x2 = data.cx + data.radius * cos(a2);
      geom::coord_t y2 = data.cy + data.radius * sin(a2);
      m_ge->create_arc_segment(&ps, x1, y1, x2, y2, tan((a2 - a1) / 4));
      add_to_plane(ps, m_current_plane->name(), geom::ObjectType::trace);
   }
   void addCircle(const DL_CircleData& data) override
   {
      geom::iShape* ps = nullptr;
      m_ge->create_circle(&ps, data.cx, data.cy, data.radius);
      add_to_plane(ps, m_current_plane->name(), geom::ObjectType::pin);
   }
   void addPolyline(const DL_PolylineData& data) override
   {
      if (m_current_shape) {
         m_current_shape->commit();
      }
      m_ge->create_shape(&m_current_shape, false, true);
      add_to_plane(m_current_shape, m_current_plane->name(), geom::ObjectType::areafill);
   }
   void addVertex(const DL_VertexData& data) override
   {
      m_current_shape->add_vertex(data.x, data.y, data.bulge);
   }
   void add3dFace(const DL_3dFaceData& data) override
   {
   }
   void addXDataString(int id, const std::string& data) override
   {
   }

} s_loader;

extern "C" BOOST_SYMBOL_EXPORT
iPcbLoader * loader()
{
   return &s_loader;
}
