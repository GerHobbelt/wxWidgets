
#include "pch.h"

#include <forward_list>

#include "assert_mgr.h"

#include "attachment_types.h"

#include "geom_type_desc.h"
#include "geom_impl.h"

#include "circle_impl.h"
#include "segment_impl.h"
#include "arc_segment_impl.h"
#include "rect_impl.h"
#include "shape_impl.h"

#include "geom_engine_base.h"

bool cGeomTypeDesc::cIter::next(iShape **ps)
{
   auto size = m_indices->size();
   if (size && (m_idx == -1 || m_idx < size - 1)) {
      auto idx = m_indices->at(++m_idx);
      auto &pImpl = (iGeomImpl *&)*ps;
      if (!pImpl) {
         pImpl = new cGeomImpl(nullptr);
      }
      pImpl->set_geom_data(m_shapes[idx].get());
      return true;
   }
   *ps = nullptr;
   return false;
}

bool cShapeImpl::cHolesIter::next(iPolygon** ps)
{
   if (m_end && (m_idx == m_beg - 1 || m_idx < m_end - 1)) {
      auto& p = (iGeomImpl*&)*ps;
      if (!p) {
         p = new cGeomImpl(nullptr);
      }
      p->set_geom_data(m_shapes[++m_idx].get());
      return true;
   }
   *ps = nullptr;
   return false;
}

struct cGeomEngine
   : public iEngine
{
   cGeomEngineBase* m_pEngine;

   cGeomEngine(cGeomEngineBase* pEngine)
      : m_pEngine(pEngine)
   {
   }

   iPlane* create_plane(int id, const shm::string::value_type* name) override
   {
      return new cPlane(m_pEngine->create_plane(id, name));
   }
   void create_circle(iShape** res, double x, double y, double radius, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = new cGeomImpl(shm::construct<cCircleImpl>(hole, filled, x, y, radius PASS_TAG));
   }
   void create_segment(iShape** res, double x1, double y1, double x2, double y2, double width = 0, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = new cGeomImpl(shm::construct<cSegmentImpl>(hole, filled, cPoint{ x1, y1 }, cPoint{ x2, y2 }, width PASS_TAG));
   }
   void create_arc_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t center_x, coord_t center_y, coord_t r,
      coord_t width, bool hole, bool filled, const char* tag) override
   {
      *res = new cGeomImpl(shm::construct<cArcSegmentImpl>(hole, filled, cPoint{ x1, y1 }, cPoint{ x2, y2 }, cPoint{ center_x, center_y }, r, width PASS_TAG));
   }
   void create_arc_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, double bulge, coord_t width, bool hole, bool filled, const char* tag) override
   {
      *res = new cGeomImpl(shm::construct<cArcSegmentImpl>(hole, filled, cPoint{ x1, y1 }, cPoint{ x2, y2 }, bulge, width PASS_TAG));
   }
   void create_rectangle(iShape** res, double left, double bottom, double right, double top, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = new cGeomImpl(shm::construct<cRectImpl>(hole, filled, left, bottom, right, top PASS_TAG));
   }
   void create_shape(iShape** res, bool hole = false, bool filled = true, const char* tag = nullptr) override
   {
      *res = new cGeomImpl(shm::construct<cShapeImpl>(iPolygon::Type::polyline, hole, filled PASS_TAG));
   }
   void create_shape(iShape** res, iShape* ps) override
   {
   }

   void expand(iShape** res, const iShape* ps, double dist) override
   {
   }
   void invert(iShape** res, const iShape* ps) override
   {
   }

   void intersect(iShape** res, const iShape* ps1, const iShape* ps2) override
   {
   }
   void merge(iShape** res, const iShape* ps1, const iShape* ps2) override
   {
   }
   void cut(iShape** res, const iShape* ps1, const iShape* ps2) override
   {
   }

   bool distance(const iShape* ps1, const iShape* ps2, double& distance, double* x1 = 0, double* y1 = 0, double* x2 = 0, double* y2 = 0) override
   {
      return false;
   }
};
