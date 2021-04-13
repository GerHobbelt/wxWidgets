#pragma once

#include <list>
#include "cavc/polyline.hpp"
#include "geom_impl_base.h"

using ptr_type = std::unique_ptr<cGeomImplBase>;
using offset_ptr_type = shm::unique_offset_ptr<cGeomImplBase>;

interface iGeomImpl : public geom::iShape
{
   virtual cGeomImplBase* geom_data() = 0;
   virtual void set_geom_data(cGeomImplBase*) = 0;
   virtual std::list<iGeomImpl*>& holes() = 0;
};

struct cGeomTypeDesc
{
   struct cSpatialIndexTraits
   {
      using Real = geom::coord_t;
      using boxes_t = shm::unique_offset_ptr<Real>;
      using indices_t = shm::unique_offset_ptr<size_t>;

      template <class T>
      using alloc = shm::template alloc<T>;

      static indices_t allocIndices(size_t size, alloc<size_t>& a)
      {
         auto p = a.allocate(size);
         return indices_t(new (&*p) size_t[size]);
      }
      static void moveIndices(indices_t& to, indices_t&& from, alloc<size_t>& a) {
         a.deallocate(to, 0);
         to = std::exchange(from, nullptr);
      }
      static boxes_t allocBoxes(size_t size, alloc<Real>& a)
      {
         auto p = a.allocate(size);
         return boxes_t(new (&*p) Real[size]);
      }
      static void moveBoxes(boxes_t& to, boxes_t&& from, alloc<Real>& a) {
         a.deallocate(to, 0);
         to = std::exchange(from, nullptr);
      }
   };

   shm::alloc<offset_ptr_type> m_alloc;

   using cSpatialIndex = cavc::StaticSpatialIndex<geom::coord_t, cSpatialIndexTraits>;

   cSpatialIndex m_index;

   using shapes_t = shm::vector<offset_ptr_type>;
   shapes_t m_shapes;

   std::list<offset_ptr_type> m_shapes_temp;

   cGeomTypeDesc()
      : m_index(1, (shm::alloc<size_t>&)m_alloc, (shm::alloc<geom::coord_t>&)m_alloc)
      , m_shapes(m_alloc)
   {
   }
   cGeomTypeDesc(cGeomTypeDesc&& x)
      : m_index(std::move(x.m_index))
      , m_shapes(std::move(x.m_shapes))
      , m_shapes_temp(std::move(x.m_shapes_temp))
   {
   }
   ~cGeomTypeDesc()
   {
   }

   struct cIter : public geom::iShapeIter
   {
      std::shared_ptr<std::vector<size_t>> m_indices;
      const shapes_t& m_shapes;
      size_t m_idx = -1;

      cIter(const shapes_t& shapes)
         : m_shapes(shapes)
      {
      }
      cIter(const cIter& x)
         : m_shapes(x.m_shapes)
         , m_indices(x.m_indices)
         , m_idx(x.m_idx)
      {
      }
      bool first(geom::iShape** ps) override
      {
         m_idx = -1;
         return next(ps);
      }
      SYMBOL_EXPORT bool next(geom::iShape **ps) override;
      size_t count() override
      {
         return m_indices->size();
      }

      geom::iShapeIter* clone() const override
      {
         return new cIter(*this);
      }
   };

   bool shapes(geom::iShapeIter** res, const geom::cRect& bounds, geom::iPlane::RetrieveOptions opt = geom::iPlane::RetrieveOptions::shape) const
   {
      auto indices = std::make_shared<std::vector<size_t>>();
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
