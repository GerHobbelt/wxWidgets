#pragma once

#include "geom_storage.h"
#include "attachment_list.h"
#include "geom_type_desc.h"

#define USE_TAG
#ifdef USE_TAG
#define TAG , const char* tag
#define PASS_TAG , tag
#else
#define TAG
#define PASS_TAG
#endif

struct cGeomImpl
   : public iShape
{
   shm::offset_ptr<cGeomTypeDesc> m_holder;

   union {
      unsigned m_holes_count = -1; // for outlines
      unsigned m_outline_idx; // for holes
   };
   bool m_hole, m_filled, m_static = false;

   cAttachmentList m_attachment;
   shm::string::allocator_type m_alloc;

#ifdef USE_TAG
   shm::string m_tag;
#endif

   cGeomImpl(bool hole, bool filled TAG, shm::string::allocator_type& a)
      : m_hole(hole)
      , m_filled(filled)
      , m_alloc(a)
#ifdef USE_TAG
      , m_tag(tag ? tag : "", a)
#endif
   {
   }

   auto get_allocator()
   {
      return m_alloc;
   }

   iAttachment* attachment(int id) const override
   {
      return m_attachment.get(id);
   }
   bool add_attachment(iAttachment* new_attachment) override
   {
      return m_attachment.add(new_attachment);
   }
   bool add_attachment(unique_ptr<iAttachment> new_attachment)
   {
      return m_attachment.add(new_attachment);
   }
   bool remove_attachment(int id) override
   {
      return m_attachment.remove(id);
   }

   bool empty() const override
   {
      return false;
   }
   bool closed() const override
   {
      return true;
   }
   bool filled() const override
   {
      return m_filled;
   }
   bool hole() const override
   {
      return m_hole;
   }

   cVertexIter vertices() const override
   {
      return cVertexIter();
   }
   bool outline(iPolygon** res) const override
   {
      return false;
   }
   void holes(iPolygonIter** res) const override
   {
   }

   cSegment segment() const override
   {
      return cSegment();
   }
   cArc arc_segment() const override
   {
      return cArc();
   }
   cCircle circle() const override
   {
      return cCircle();
   }
   cRect rectangle() const override
   {
      return cRect();
   }

   bool equal(const iPolygon* ps, double tolerance = 0) const override
   {
      return false;
   }

   bool includes(const iPolygon* ps, bool open_set = false) const override
   {
      return false;
   }
   bool overlaps(const iPolygon* ps, bool open_set = false) const override
   {
      return false;
   }
   bool touches(const iPolygon* ps) const override
   {
      return false;
   }
   double length() const override
   {
      return 0;
   }
   double area() const override
   {
      return 0;
   }

   void add_vertex(double x, double y, coord_t bulge) override
   {
   }
   bool add_arc(coord_t center_x, coord_t center_y, coord_t r, coord_t x, coord_t y, bool ccw = true) override
   {
      return false;
   }
   bool add_hole(iPolygon* hole) override
   {
      return false;
   }
   void commit() override
   {
   }
};
