
#pragma once

#include "interface.h"
#include <memory>
#include <functional>
#include <numbers>
#include <cmath>

namespace geom {

#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

   using coord_t = double;
   inline constexpr double pi() { return double(3.14159265358979323846264338327950288); };
   inline constexpr auto inf = std::numeric_limits<coord_t>::infinity();

   template <typename T = void>
   struct tPoint
   {
      coord_t m_x = 0, m_y = 0;

      tPoint()
      {
      }
      tPoint(const tPoint & x)
         : m_x(x.m_x), m_y(x.m_y)
      {
      }
      tPoint(coord_t x, coord_t y)
         : m_x(x), m_y(y)
      {
      }
      tPoint operator + (const tPoint& x) const noexcept
      {
         return { m_x + x.m_x, m_y + x.m_y };
      }
      tPoint operator - (const tPoint& x) const noexcept
      {
         return { m_x - x.m_x, m_y - x.m_y };
      }
      tPoint operator += (const tPoint& x) noexcept
      {
         m_x += x.m_x;
         m_y += x.m_y;
         return *this;
      }
      tPoint operator -= (const tPoint& x) noexcept
      {
         m_x -= x.m_x;
         m_y -= x.m_y;
         return *this;
      }
      tPoint operator - () const noexcept
      {
         return { -m_x, -m_y };
      }
      tPoint operator * (double x) const noexcept
      {
         return { m_x * x, m_y * x };
      }
      tPoint operator / (double x) const noexcept
      {
         return { m_x / x, m_y / x };
      }
      coord_t operator * (const tPoint& x) const noexcept // dot product
      {
         return m_x * x.m_x + m_y * x.m_y;
      }
      coord_t operator % (const tPoint& x) const noexcept // cross product
      {
         return m_x * x.m_y - m_y * x.m_x;
      }
      coord_t length() const noexcept
      {
         return std::sqrt(*this * *this);
      }
      tPoint mirror_x() const noexcept
      {
         return { m_x, -m_y };
      }
   };

   using cPoint = tPoint<>;

   template <typename T = void>
   struct tRect
   {
      coord_t m_left = inf, m_bottom = inf, m_right = -inf, m_top = -inf;

      tRect() noexcept
      {
      }
      tRect(const tRect& x) noexcept
         : tRect(x.m_left, x.m_bottom, x.m_right, x.m_top)
      {
      }
      tRect(const tPoint<T>& c, coord_t cx, coord_t cy) noexcept
         : tRect(c.m_x - cx / 2, c.m_y - cy / 2, c.m_x + cx / 2, c.m_y + cy / 2)
      {
      }
      tRect(const tPoint<T>& lb, const tPoint<T>& rt) noexcept
         : tRect(lb.m_x, lb.m_y, rt.m_x, rt.m_y)
      {
      }
      tRect(coord_t left, coord_t bottom, coord_t right, coord_t top) noexcept
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

      tPoint<T> top_left() const noexcept
      {
         return { m_left, m_top };
      }
      tPoint<T> right_bottom() const noexcept
      {
         return { m_right, m_bottom };
      }
      tPoint<T> center() const noexcept
      {
         return { (m_left + m_right) / 2, (m_top + m_bottom) / 2 };
      }

      tRect& offset(const tPoint<T>& o) noexcept
      {
         m_left += o.m_x;
         m_right += o.m_x;
         m_top += o.m_y;
         m_bottom += o.m_y;
         return *this;
      }
      tRect& inflate(coord_t dx, coord_t dy) noexcept
      {
         m_left -= dx;
         m_right += dx;
         m_bottom -= dy;
         m_top += dy;
         return *this;
      }

      tRect& normalize() noexcept
      {
         if (m_left > m_right) {
            std::swap(m_left, m_right);
         }
         if (m_bottom > m_top) {
            std::swap(m_bottom, m_top);
         }
         return *this;
      }

      tRect rectangle() const noexcept
      {
         return tRect(*this).normalize();
      }

      void operator += (const tRect& x)
      {
         m_top = std::max(m_top, x.m_top);
         m_left = std::min(m_left, x.m_left);
         m_right = std::max(m_right, x.m_right);
         m_bottom = std::min(m_bottom, x.m_bottom);
      }
   };

   using cRect = tRect<>;

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
         return length() * m_width + pi() * m_width * m_width / 4;
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
   {
      double m_bulge = 0;

      cArc()
         : cSegment()
      {
      }
      cArc(const cArc& x)
         : cSegment(x), m_bulge(x.m_bulge)
      {
      }
      cArc(const cPoint& beg, const cPoint& end, coord_t bulge, coord_t width = 0)
         : cSegment(beg, end, width), m_bulge(bulge)
      {
      }
      cArc(const cPoint& beg, const cPoint& end, const cPoint& center, coord_t radius, coord_t width = 0)
         : cSegment(beg, end, width)
      {
         m_bulge = calc_bulge(beg, end, center, abs(radius), radius > 0);
      }
      cArc(coord_t beg_x, coord_t beg_y, coord_t end_x, coord_t end_y, coord_t center_x, coord_t center_y, coord_t radius, coord_t width = 0)
         : cSegment({beg_x, beg_y}, {end_x, end_y}, width)
      {
         m_bulge = calc_bulge(m_beg, m_end, { center_x, center_y }, abs(radius), radius > 0);
      }

      bool is_ccw() const noexcept
      {
         return m_bulge > 0;
      }
      bool is_cw() const noexcept
      {
         return m_bulge < 0;
      }

      cCircle center_and_radius() const noexcept
      {
         double bulge = std::abs(m_bulge);
         auto v = m_end - m_beg;
         auto d = v.length();
         coord_t radius = d * (bulge * bulge + 1) / (4 * bulge);

         coord_t sagitta = bulge * d / 2;
         coord_t median = radius - sagitta;
         auto center_offset = cPoint(-v.m_y, v.m_x) * (median / d);
         if (m_bulge < 0) {
            center_offset = -center_offset;
         }

         cPoint center = m_beg + v / 2 + center_offset;
         return { center, radius };
      }

      double start_angle() const noexcept
      {
         auto bc = m_beg - center_and_radius().m_center;
         return atan2(bc.m_y, bc.m_x);
      }
      double sweep() const noexcept
      {
         return 4 * atan(m_bulge);
      }

      cRect rectangle() const noexcept
      {
         cRect retval = cSegment::rectangle();
#if 1
         // Approximate bounding box for arc segments (as in CavalierContours)
         // This approximate bounding box is always equal to or bigger than the true bounding box
         auto sagitta = cPoint(m_end.m_y - m_beg.m_y, m_beg.m_x - m_end.m_x) * m_bulge / 2;
         auto offset_rect = cRect(retval).offset(sagitta);
         retval += offset_rect;
#else
         cCircle cir = center_and_radius();
         cPoint db = m_beg - cir.m_center;
         cPoint de = m_end - cir.m_center;
         bool sbx = db.m_x > 0, sex = de.m_x > 0;
         bool sby = db.m_y > 0, sey = de.m_y > 0;
         if (sby == sey) {
            if (abs(m_bulge) > 1) {
               retval.m_left = cir.m_center.m_x - cir.m_radius;
               retval.m_right = cir.m_center.m_x + cir.m_radius;
            }
         }
         else if (sby != is_ccw()) {
            retval.m_right = cir.m_center.m_x + cir.m_radius;
         }
         else {
            retval.m_left = cir.m_center.m_x - cir.m_radius;
         }

         if (sbx == sex) {
            if (abs(m_bulge) > 1) {
               retval.m_bottom = cir.m_center.m_x - cir.m_radius;
               retval.m_top = cir.m_center.m_x + cir.m_radius;
            }
         }
         else if (sbx == is_ccw()) {
            retval.m_top = cir.m_center.m_x + cir.m_radius;
         }
         else {
            retval.m_bottom = cir.m_center.m_x - cir.m_radius;
         }
#endif
         return retval;
      }

   protected:
      static double calc_bulge(const cPoint& v1, const cPoint& v2, const cPoint& center, double r, bool ccw)
      {
         // <image src="bulge.png"/>
 
         //tex:
         //Bulge is defined as $(2s/d)$, where $d = |\vec v_2 - \vec v_1|$ is the chord, and s1,2 are the sagittas of the corresponding arcs. Thus: $$r^2 = (r - s)^2 + d^2/4$$
         //$$s^2 - 2rs + d^2/4 = 0$$
         //$$(2s/d)^2 - 4(r/d)(2s/d) + 1 = 0$$
         //Let a = $2r/d$, then
         //$$bulge = (a \pm \sqrt{a^2 - 1})$$
         //Let bulge1 and bulge2 be the roots corresponding to s1 and s2. Note that $bulge1 \cdot bulge2 = 1$

         // for ccw arcs, we need the lesser root if the center point is to the left from V1->V2, and the larger one otherwise
         // for cw arcs, it is wise versa

         auto vv = v2 - v1, cv = center - v1;
         double a = 2 * r / sqrt(vv * vv);
         double bulge = a + sqrt(abs(a * a - 1));
         bool center_to_the_left = vv % cv > 0; // center point is to the left from V1->V2
         if (center_to_the_left == ccw) {
            bulge = 1.0 / bulge;
         }
         return ccw ? bulge : -bulge;
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
      virtual ~iShapeIter() {}
      virtual bool first(iShape** ps) = 0;
      virtual bool next(iShape** ps) = 0;
      virtual size_t count() = 0;
      virtual iShapeIter* clone() const = 0;
   };
   interface iPolygonIter
      //: public iSharedObject
   {
      virtual ~iPolygonIter() {}
      virtual bool first(iPolygon** pc) = 0;
      virtual bool next(iPolygon** pc) = 0;
      virtual size_t count() = 0;
      virtual iPolygonIter* clone() const = 0;
   };
   interface iVertexIter
      //: public iSharedObject
   {
      virtual ~iVertexIter() {}
      virtual bool first(coord_t& vertex_x, coord_t& vertex_y) = 0;
      virtual bool next(coord_t& vertex_x, coord_t& vertex_y) = 0;
      virtual double bulge() = 0;
      virtual iVertexIter* clone() const = 0;
      virtual size_t count() = 0;
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
            m_vertex.m_bulge = m_iter->bulge();
            m_arc = (m_vertex.m_bulge != 0);
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
         m_vertex.m_bulge = m_iter->bulge();
         m_arc = (m_vertex.m_bulge != 0);
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

   interface iPolygon
   {
      enum class Type : std::int8_t { unknown, circle, segment, arc_segment, rectangle, polyline };

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
      virtual void release() = 0;
      virtual iShape* clone() = 0;

      virtual bool outline(iPolygon** res) const = 0;
      virtual void holes(iPolygonIter** res) const = 0;

      virtual bool add_hole(iPolygon* hole) = 0;
   };

   interface iTransform : public iShape
   {
      virtual void add_offset(coord_t offset_x, coord_t offset_y) = 0;
      virtual void add_rotation(coord_t center_x, coord_t center_y, coord_t angle) = 0;
   };


   class cShapeIter
   {
   protected:
      std::unique_ptr<iShapeIter> m_iter;

      struct cShapeReleaser
      {
         void operator()(iShape* p)
         {
            p->release();
         }
      };
      std::unique_ptr<iShape, cShapeReleaser> m_shape;

   public:
      cShapeIter(iShapeIter* iter = nullptr) noexcept
         : m_iter(iter)
      {
         if (iter) {
            iShape* shape = nullptr;
            m_iter->first(&shape);
            m_shape.reset(shape);
         }
      }
      cShapeIter(const cShapeIter& x) noexcept
         : m_iter(x.m_iter ? x.m_iter->clone() : nullptr)
      {
         m_shape.reset(x.m_shape ? x.m_shape->clone() : nullptr);
      }
      cShapeIter(cShapeIter&& x) noexcept
         : m_iter(std::move(x.m_iter))
         , m_shape(std::move(x.m_shape))
      {
      }
      const iShape& operator * () const noexcept
      {
         return *m_shape;
      }
      const iShape* operator -> () const noexcept
      {
         return m_shape.get();
      }
      cShapeIter& operator ++ () noexcept
      {
         iShape* shape = m_shape.get();
         m_iter->next(&shape);
         if (shape != m_shape.get()) {
            m_shape.reset(shape);
         }
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

      virtual void clear() = 0;

      virtual void create_circle(iShape** res, coord_t x, coord_t y, coord_t radius, bool hole = false, bool filled = true, const char * tag = nullptr) = 0;
      virtual void create_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t width = 0, bool hole = false, bool filled = true, const char* tag = nullptr) = 0;
      virtual void create_arc_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t center_x, coord_t center_y, coord_t r, coord_t width = 0, bool hole = false, bool filled = true, const char* tag = nullptr) = 0;
      virtual void create_arc_segment(iShape** res, coord_t x1, coord_t y1, coord_t x2, coord_t y2, double bulge, coord_t width = 0, bool hole = false, bool filled = true, const char* tag = nullptr) = 0;
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
