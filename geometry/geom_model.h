
#pragma once

#include "interface.h"
#include <memory>
#include <numbers>
#include <cmath>

namespace geom {

#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

   using coord_t = double;
   constexpr double pi() { return double(3.14159265358979323846264338327950288); };

   struct cPoint
   {
      coord_t m_x = 0, m_y = 0;

      cPoint()
      {
      }
      cPoint(const cPoint & x)
         : m_x(x.m_x), m_y(x.m_y)
      {
      }
      cPoint(coord_t x, coord_t y)
         : m_x(x), m_y(y)
      {
      }
      cPoint operator + (const cPoint& x) const noexcept
      {
         return { m_x + x.m_x, m_y + x.m_y };
      }
      cPoint operator - (const cPoint& x) const noexcept
      {
         return { m_x - x.m_x, m_y - x.m_y };
      }
      cPoint operator += (const cPoint& x) noexcept
      {
         m_x += x.m_x;
         m_y += x.m_y;
         return *this;
      }
      cPoint operator -= (const cPoint& x) noexcept
      {
         m_x -= x.m_x;
         m_y -= x.m_y;
         return *this;
      }
      coord_t dot_prod(const cPoint& x) const noexcept
      {
         return m_x * x.m_x + m_y * x.m_y;
      }
      coord_t cross_prod(const cPoint& x) const noexcept
      {
         return m_x * x.m_y - m_y * x.m_x;
      }
      coord_t length() const noexcept
      {
         return std::sqrt(length2());
      }
      coord_t length2() const noexcept
      {
         return dot_prod(*this);
      }
   };
   struct cRect
   {
      coord_t m_left = 0, m_bottom = 0, m_right = 0, m_top = 0;

      cRect() noexcept
      {
         constexpr auto inf = std::numeric_limits<coord_t>::infinity();
         m_left = inf, m_bottom = inf, m_right = -inf, m_top = -inf;
      }
      cRect(const cRect& x) noexcept
         : m_left(x.m_left), m_bottom(x.m_bottom), m_right(x.m_right), m_top(x.m_top)
      {
      }
      cRect(const cPoint& lb, const cPoint& ut) noexcept
         : m_left(lb.m_x), m_bottom(lb.m_y), m_right(ut.m_x), m_top(ut.m_y)
      {
      }
      cRect(coord_t left, coord_t bottom, coord_t right, coord_t top) noexcept
         : m_left(left), m_bottom(bottom), m_right(right), m_top(top)
      {
      }

      coord_t width() const noexcept
      {
         return m_right - m_left;
      }
      coord_t height() const noexcept
      {
         return m_top - m_bottom;
      }
      double area() const noexcept
      {
         return height() * width();
      }

      cPoint center() const noexcept
      {
         return { (m_left + m_right) / 2, (m_top + m_bottom) / 2 };
      }

      cRect& inflate(coord_t dx, coord_t dy) noexcept
      {
         m_left -= dx;
         m_right += dx;
         m_bottom -= dy;
         m_top += dy;
         return *this;
      }

      cRect& normalize() noexcept
      {
         if (m_left > m_right) {
            std::swap(m_left, m_right);
         }
         if (m_bottom > m_top) {
            std::swap(m_bottom, m_top);
         }
         return *this;
      }

      cRect rectangle() const noexcept
      {
         return cRect(*this).normalize();
      }

      void operator += (const cRect& x)
      {
         m_top = std::max(m_top, x.m_top);
         m_left = std::min(m_left, x.m_left);
         m_right = std::max(m_right, x.m_right);
         m_bottom = std::min(m_bottom, x.m_bottom);
      }
   };
   struct cSegment
   {
      cPoint m_beg, m_end;
      coord_t m_width;

      cSegment()
      {
      }
      cSegment(const cSegment& x)
         : m_beg(x.m_beg)
         , m_end(x.m_end)
         , m_width(x.m_width)
      {
      }
      cSegment(const cPoint& beg, const cPoint& end, coord_t width = 0)
         : m_beg(beg)
         , m_end(end)
         , m_width(width)
      {
      }

      cPoint beg() const noexcept
      {
         return m_beg;
      }
      cPoint end() const noexcept
      {
         return m_end;
      }

      coord_t length() const noexcept
      {
         return (m_end - m_beg).length();
      }
      double area() const noexcept
      {
         return length() * m_width + std::numbers::pi * m_width * m_width / 4;
      }
      coord_t width() const noexcept
      {
         return m_width;
      }
      cRect rectangle() const noexcept
      {
         return cRect(m_beg, m_end).normalize().inflate(m_width / 2, m_width / 2);
      }
   };
   struct cCircle
   {
      cPoint m_center;
      coord_t m_radius = 0;

      cCircle()
      {
      }
      cCircle(const cCircle & x)
         : m_center(x.m_center), m_radius(x.m_radius)
      {
      }
      cCircle(const cPoint& center, coord_t radius)
         : m_center(center), m_radius(radius)
      {
      }
      cCircle(coord_t center_x, coord_t center_y, coord_t radius)
         : m_center(center_x, center_y), m_radius(radius)
      {
      }

      cRect rectangle() const noexcept
      {
         auto r = abs(m_radius);
         cPoint d(r, r);
         return {m_center - d, m_center + d};
      }
   };
   struct cArc
      : public cSegment
      , public cCircle
   {
      cArc()
         : cCircle()
      {
      }
      cArc(const cArc& x)
         : cCircle(x), cSegment(x)
      {
      }
      cArc(const cPoint& beg, const cPoint& end, const cPoint& center, coord_t radius, coord_t width = 0)
         : cCircle(center, radius), cSegment(beg, end, width)
      {
      }
      cArc(coord_t beg_x, coord_t beg_y, coord_t end_x, coord_t end_y, coord_t center_x, coord_t center_y, coord_t radius)
         : cCircle(center_x, center_y, radius), cSegment({beg_x, beg_y}, {end_x, end_y})
      {
      }
   };

   interface iPlane;
   interface iShape;
   interface iPolygon;

   //interface iSharedObject
   //{
   //   virtual size_t addref() = 0;
   //   virtual size_t release() = 0;
   //};

   //template <class T, class D = std::default_delete<T>>
   //class cSharedObject : public T
   //{
   // protected:
   //    std::atomic_size_t m_count;

   //public:
   //    size_t addref() override
   //    {
   //       return ++m_count;
   //    }
   //    size_t release() override
   //    {
   //       if (--m_count == 0) {
   //          D()(this);
   //       }
   //       return m_count;
   //    }
   //};

   interface iShapeIter
      //: public iSharedObject
   {
      virtual bool first(iShape** ps) = 0;
      virtual bool next(iShape** ps) = 0;
      virtual size_t count() = 0;
      virtual iShapeIter* clone() const = 0;
   };
   interface iPolygonIter
      //: public iSharedObject
   {
      virtual bool first(iPolygon** pc) = 0;
      virtual bool next(iPolygon** pc) = 0;
      virtual size_t count() = 0;
      virtual iPolygonIter* clone() const = 0;
   };
   interface iVertexIter
      //: public iSharedObject
   {
      virtual bool first(coord_t& vertex_x, coord_t& vertex_y) = 0;
      virtual bool next(coord_t& vertex_x, coord_t& vertex_y) = 0;
      virtual bool arc(coord_t& center_x, coord_t& center_y, coord_t& radius) = 0;
      virtual iVertexIter* clone() const = 0;
      virtual size_t count() = 0;
   };

   class cShapeIter
   {
   protected:
      std::unique_ptr<iShapeIter> m_iter;
      iShape* m_shape = nullptr;

   public:
      cShapeIter(iShapeIter* iter = nullptr) noexcept
         : m_iter(iter)
      {
         if (iter) {
            m_iter->first(&m_shape);
         }
      }
      cShapeIter(const cShapeIter& x) noexcept
         : m_iter(x.m_iter ? x.m_iter->clone() : nullptr)
         , m_shape(x.m_shape)
      {
      }
      cShapeIter(cShapeIter&& x) noexcept
         : m_iter(std::move(x.m_iter))
         , m_shape(x.m_shape)
      {
      }
      const iShape& operator * () const noexcept
      {
         return *m_shape;
      }
      const iShape* operator -> () const noexcept
      {
         return m_shape;
      }
      cShapeIter& operator ++ () noexcept
      {
         m_iter->next(&m_shape);
         return *this;
      }
      bool operator == (const cShapeIter& x) noexcept
      {
         return m_shape == x.m_shape;
      }
      operator bool() const noexcept
      {
         return !!m_shape;
      }
      size_t count() const noexcept
      {
         return m_iter->count();
      }
   };

   class cVertexIter
   {
   protected:
      cArc m_vertex;
      std::unique_ptr<iVertexIter> m_iter;
      bool m_valid = false, m_arc = false;

   public:
      cVertexIter(iVertexIter* iter = nullptr)
         : m_iter(iter)
      {
         if (iter) {
            m_valid = m_iter->first(m_vertex.m_beg.m_x, m_vertex.m_beg.m_y);
            m_arc = m_iter->arc(m_vertex.m_center.m_x, m_vertex.m_center.m_y, m_vertex.m_radius);
            m_valid = m_iter->next(m_vertex.m_end.m_x, m_vertex.m_end.m_y);
         }
      }
      cVertexIter(const cVertexIter& x)
         : m_iter(x.m_iter ? x.m_iter->clone() : nullptr)
         , m_vertex(x.m_vertex)
         , m_valid(x.m_valid)
         , m_arc(x.m_arc)
      {
      }
      cVertexIter(cVertexIter&& x) noexcept
         : m_iter(std::move(x.m_iter))
         , m_vertex(x.m_vertex)
         , m_valid(x.m_valid)
         , m_arc(x.m_arc)
      {
      }
      const cArc& operator * ()
      {
         return m_vertex;
      }
      const cArc* operator -> ()
      {
         return &m_vertex;
      }
      cVertexIter& operator ++ ()
      {
         m_vertex.m_beg = m_vertex.m_end;
         m_arc = m_iter->arc(m_vertex.m_center.m_x, m_vertex.m_center.m_y, m_vertex.m_radius);
         m_valid = m_iter->next(m_vertex.m_end.m_x, m_vertex.m_end.m_y);
         return *this;
      }
      bool operator == (const cVertexIter& x)
      {
         if (!m_valid) {
            return !x.m_valid;
         }
         return m_iter == x.m_iter;
      }
      operator bool() const noexcept
      {
         return m_valid;
      }
      bool is_arc() const
      {
         return m_arc;
      }
   };

   template <typename T>
   class range
   {
      T m_beg, m_end;

   public:
      range()
      {
      }
      range(const range & r)
         : m_beg(r.m_beg)
         , m_end(r.m_end)
      {
      }
      range(range&& r)
         : m_beg(std::move(r.m_beg))
         , m_end(std::move(r.m_end))
      {
      }
      range(T&& x)
         : m_beg(std::move(x))
      {
      }
      template <class ...U>
      range(U&&... args)
         : m_beg(args...)
      {
      }

      void operator = (const range& r)
      {
         m_beg = r.m_beg;
         m_end = r.m_end;
      }
      void operator = (range&& r)
      {
         m_beg = std::move(r.m_beg);
         m_end = std::move(r.m_end);
      }

      T & begin() &
      {
         return m_beg;
      }
      const T & end() const&
      {
         return m_end;
      }
      T && begin() &&
      {
         return std::move(m_beg);
      }
      T&& end() &&
      {
         return std::move(m_end);
      }
   };

   enum class ObjectType {
      trace, pin, via, areafill,
      count
   };

   interface iAttachment
   {
      virtual int id() const = 0;
   };

   interface iPolygon
   {
      enum class Type { unknown, circle, segment, arc_segment, rectangle, polyline };

      virtual Type type() const = 0;

      virtual bool empty() const = 0;
      virtual bool closed() const = 0;
      virtual bool filled() const = 0;
      virtual bool hole() const = 0;

      virtual coord_t length() const = 0;
      virtual coord_t area() const = 0;

      virtual cVertexIter vertices() const = 0;

      virtual cCircle circle() const = 0;
      virtual cSegment segment() const = 0;
      virtual cArc arc_segment() const = 0;
      virtual cRect rectangle() const = 0;

      virtual iAttachment* attachment(int id) const = 0;
      virtual bool add_attachment(iAttachment* new_attachment) = 0;
      virtual bool remove_attachment(int id) = 0;

      virtual bool equal(const iPolygon* ps, coord_t tolerance = 0) const = 0;

      virtual bool includes(const iPolygon* ps, bool open_set = false) const = 0;
      virtual bool overlaps(const iPolygon* ps, bool open_set = false) const = 0;
      virtual bool touches(const iPolygon* ps) const = 0;

      virtual void add_vertex(coord_t x, coord_t y, coord_t bulge = 0) = 0;
      virtual bool add_arc(coord_t center_x, coord_t center_y, coord_t r, coord_t x, coord_t y, bool ccw = true) = 0;
      virtual void commit() = 0;
   };

   interface iShape : public iPolygon
   {
      virtual bool outline(iPolygon** res) const = 0;
      virtual void holes(iPolygonIter** res) const = 0;

      virtual bool add_hole(iPolygon* hole) = 0;
   };

   interface iTransform : public iShape
   {
      virtual void add_offset(coord_t offset_x, coord_t offset_y) = 0;
      virtual void add_rotation(coord_t center_x, coord_t center_y, coord_t angle) = 0;
   };

   interface iPlane
   {
      enum class RetrieveOptions { shape, group, contour };

      virtual int id() const = 0;
      virtual const char * name() const = 0;

      virtual void add_shape(iShape* ps, ObjectType type) = 0;
      virtual void remove_shape(iShape* ps) = 0;

      virtual cShapeIter shapes(const cRect& bounds, ObjectType type, RetrieveOptions opt = RetrieveOptions::shape) const = 0;

      virtual cRect bounds() = 0;
   };

   interface iEngine
   {
      virtual iPlane* create_plane(int id = 0, const char* name = 0) = 0;
      virtual iPlane* plane(const char* name) = 0;
      virtual iPlane* plane(size_t id) = 0;
      virtual size_t planes() const = 0;

      virtual void create_circle(iShape** res, coord_t x, coord_t y, coord_t radius, bool hole = false, bool filled = true, const char * tag = nullptr) = 0;
      virtual void create_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t width = 0, bool hole = false, bool filled = true, const char* tag = nullptr) = 0;
      virtual void create_arc_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t center_x, coord_t center_y, coord_t r, bool ccw = true, coord_t width = 0, bool hole = false, bool filled = true, const char* tag = nullptr) = 0;
      virtual void create_rectangle(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, bool hole = false, bool filled = true, const char* tag = nullptr) = 0;
      virtual void create_shape(iShape** res, bool hole = false, bool filled = true, const char* tag = nullptr) = 0;
      virtual void create_shape(iShape** res, iShape* ps) = 0;

      virtual void expand(iShape** res, const iShape* ps, coord_t dist) = 0;
      virtual void invert(iShape** res, const iShape* ps) = 0;

      virtual void intersect(iShape** res, const iShape* ps1, const iShape* ps2) = 0;
      virtual void merge(iShape** res, const iShape* ps1, const iShape* ps2) = 0;
      virtual void cut(iShape** res, const iShape* ps1, const iShape* ps2) = 0;

      virtual bool distance(const iShape* ps1, const iShape* ps2, coord_t& distance, coord_t* x1 = 0, coord_t* y1 = 0, coord_t* x2 = 0, coord_t* y2 = 0) = 0;
   };

#pragma pop_macro("max")
#pragma pop_macro("min")

}; // namespace geom
