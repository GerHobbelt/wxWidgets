#pragma once

#include "hole_attachment.h"

using ptr_type = unique_ptr<iPolygon>;
using offset_ptr_type = shm::unique_offset_ptr<iPolygon>;

namespace {
}

struct cGeomTypeDesc
{
   struct cSpatialIndexTraits
   {
      using Real = geom::coord_t;
      using boxes_t = shm::unique_offset_ptr<Real>;
      using indices_t = shm::unique_offset_ptr<size_t>;

      static indices_t allocIndices(size_t size)
      {
         return indices_t(shm::construct_array<size_t>(size));
      }
      static void moveIndices(indices_t& to, indices_t&& from) {
         shm::alloc<size_t>().deallocate(to, 0);
         to = exchange(from, nullptr);
      }
      static boxes_t allocBoxes(size_t size)
      {
         return boxes_t(shm::construct_array<Real>(size));
      }
      static void moveBoxes(boxes_t& to, boxes_t&& from) {
         shm::alloc<Real>().deallocate(to, 0);
         to = exchange(from, nullptr);
      }
   };

   using cSpatialIndex = cavc::StaticSpatialIndex<coord_t, cSpatialIndexTraits>;

   cSpatialIndex m_index;
   list<offset_ptr_type> m_shapes_temp;

   using shapes_t = shm::vector<offset_ptr_type>;
   shapes_t m_shapes;

   cGeomTypeDesc()
      : m_index(1)
      , m_shapes(shm::alloc<offset_ptr_type>())
   {
   }
   cGeomTypeDesc(cGeomTypeDesc&& x)
      : m_index(move(x.m_index))
      , m_shapes_temp(move(x.m_shapes_temp))
      , m_shapes(move(x.m_shapes))
   {
   }
   ~cGeomTypeDesc()
   {
   }

   void add_shape(iShape* ps)
   {
      m_shapes_temp.push_back(offset_ptr_type(ps));
   }

   void commit()
   {
      auto size = m_shapes_temp.size();
      for (auto& shape : m_shapes_temp) {
         if (auto p = (cHoleAttachment*)shape->attachment(AttachmentType_Hole)) {
            size += p->m_holes.size();
         }
      }
      if (size) {
         m_shapes.reserve(m_shapes.size() + size);
         cSpatialIndex index(size);
         auto add = [this, &index](auto& shape) -> auto& {
            cRect bounds = shape->rectangle();
            index.add(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top);
            m_shapes.emplace_back(shape.get());
            return m_shapes.back();
         };
         for (auto& shape : m_shapes_temp) {
            auto& static_shape = add(shape);
            if (auto p = (cHoleAttachment*)static_shape->attachment(AttachmentType_Hole)) {
               ranges::for_each(p->m_holes, add);
               static_shape->remove_attachment(AttachmentType_Hole);
            }
         }
         index.finish();
         m_index = move(index);
         m_shapes_temp.clear();
      }
   }

   struct cIter : public iShapeIter
   {
      shared_ptr<vector<size_t>> m_indices;
      shapes_t& m_shapes;
      size_t m_idx = -1;

      cIter(shapes_t& shapes)
         : m_shapes(shapes)
      {
      }
      cIter(const cIter& x)
         : m_shapes(x.m_shapes)
         , m_indices(x.m_indices)
         , m_idx(x.m_idx)
      {
      }
      bool first(iShape** ps) override
      {
         m_idx = -1;
         return next(ps);
      }
      bool next(iShape** ps) override
      {
         auto size = m_indices->size();
         if (size && (m_idx == -1 || m_idx < size - 1)) {
            auto idx = m_indices->at(++m_idx);
            *ps = (iShape*)m_shapes[idx].get(); //TBD
            return true;
         }
         *ps = nullptr;
         return false;
      }
      size_t count() override
      {
         return m_indices->size();
      }

      iShapeIter* clone() const override
      {
         return new cIter(*this);
      }
   };

   bool shapes(iShapeIter** res, const cRect& bounds, iPlane::RetrieveOptions opt = iPlane::RetrieveOptions::shape)
   {
      commit();
      auto indices = make_shared<vector<size_t>>();
      m_index.query(bounds.m_left, bounds.m_bottom, bounds.m_right, bounds.m_top, *indices);
      if (indices->size()) {
         sort(indices->begin(), indices->end());
         auto pRes = new cIter(m_shapes);
         pRes->m_indices = indices;
         *res = pRes;
         return true;
      }
      *res = nullptr;
      return false;
   }
};
