#pragma once

#include "hole_attachment.h"
#include "geom_impl_base.h"

using ptr_type = unique_ptr<cGeomImplBase>;
using offset_ptr_type = shm::unique_offset_ptr<cGeomImplBase>;

interface iGeomImpl : public iShape
{
   virtual cGeomImplBase* geom_data() = 0;
   virtual void set_geom_data(cGeomImplBase*) = 0;
};

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

   using shapes_t = shm::vector<offset_ptr_type>;
   shapes_t m_shapes;

   cGeomTypeDesc()
      : m_index(1)
      , m_shapes(shm::alloc<offset_ptr_type>())
   {
   }
   cGeomTypeDesc(cGeomTypeDesc&& x)
      : m_index(move(x.m_index))
      , m_shapes(move(x.m_shapes))
   {
   }
   ~cGeomTypeDesc()
   {
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
      bool next(iShape** ps) override;
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
