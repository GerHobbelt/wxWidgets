
#include "pch.h"
#include "GeoLib.h"

using namespace std;
using namespace geom;

template <class T>
struct cShapeImpl
   : public iShape
   , public T
{
   Type m_type;
   bool m_hole, m_filled;
   iPlane* m_plane = nullptr;

   void* operator new(size_t)
   {
      return CMemoryPool<cShapeImpl>::Allocate();
   }
   void operator delete(void* p)
   {
      CMemoryPool<cShapeImpl>::Deallocate(p);
   }
   void* operator new(size_t, const char*, int)
   {
      return CMemoryPool<cShapeImpl>::Allocate();
   }

   template <typename ...TARGS>
   cShapeImpl(Type type, bool hole, bool filled, TARGS... args)
      : m_type(type)
      , m_hole(hole)
      , m_filled(filled)
      , T(args...)
   {
   }

   Type type() const override
   {
      return m_type;
   }

   bool empty() const override
   {
      return false;
   }
   bool closed() const override
   {
      return false;
   }
   bool filled() const override
   {
      return m_filled;
   }
   bool hole() const override
   {
      return m_hole;
   }

   void iter(iVertexIter** res) override
   {
   }

   void holes(iPolygonIter** res) const override
   {
   }

   bool outline(iPolygon** res) const override
   {
      return false;
   }

   double length() const override
   {
      return 0.0;
   }
   double area() const override
   {
      return 0.0;
   }
   bool bounds(double& x1, double& y1, double& x2, double& y2) const override
   {
      return false;
   }

   void bounding_box(iPolygon** res) const override
   {
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

   void add_vertex(double x, double y) override
   {
   }
   bool add_arc(double center_x, double center_y, double radius) override
   {
      return false;
   }
   bool add_hole(const iShape* hole) override
   {
      return false;
   }
   void commit() override
   {
   }
};

struct cPlane
   : public iPlane
{
   int m_id;
   string m_name;

   //using ptr_type = iShape *;
   using ptr_type = unique_ptr<iShape>;
   map<int, list<ptr_type>> m_shapes;

   cPlane(int a_id, const char* a_name)
      : m_id(a_id)
      , m_name(a_name)
   {
   }

   int id() const override
   {
      return m_id;
   }

   void add_shape(iShape* ps, int type) override
   {
      m_shapes[type].push_back(ptr_type(ps));
   }
   void remove_shape(iShape* ps) override
   {
   }
   bool shapes(iShapeIter** res, double xmin, double ymin, double xmax, double ymax, int type, RetrieveOptions opt = RetrieveOptions::shape) const override
   {
      return false;
   }
};

struct cGeomEngine
   : public iEngine
{
   vector<unique_ptr<cPlane>> m_planes;
   map<string, cPlane*> m_plane_dict;

   void create_plane(iPlane** res, int id, const char* name) override
   {
      m_planes.push_back(make_unique<cPlane>(id, name));
      auto& p = m_planes.back();
      *res = p.get();
      m_plane_dict[p->m_name] = p.get();
   }
   bool plane(iPlane** res, const char* name) override
   {
      auto it = m_plane_dict.find(name);
      if (it != m_plane_dict.end()) {
         *res = it->second;
         return true;
      }
      return false;
   }
   bool plane(iPlane** res, int id) override
   {
      for (auto& p : m_planes) {
         if (p->m_id == id) {
            *res = p.get();
            return true;
         }
      }
      return false;
   }

   void create_circle(iShape** res, double x, double y, double radius, bool hole = false, bool filled = true) override
   {
      *res = new cShapeImpl<C2DCircle>(iShape::Type::circle, hole, filled, C2DPoint(x, y), radius);
   }
   void create_segment(iShape** res, double x1, double y1, double x2, double y2, double radius = 0, bool hole = false, bool filled = true) override
   {
   }
   void create_rectangle(iShape** res, double x1, double y1, double x2, double y2, bool hole = false, bool filled = true) override
   {
      *res = new cShapeImpl<C2DRect>(iShape::Type::circle, hole, filled, x1, y1, x2, y2);
   }
   void create_shape(iShape** res, bool hole = false, bool filled = true) override
   {
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

static cGeomEngine s_ge;

BOOST_SYMBOL_EXPORT
iEngine* GetGeomEngine()
{
   return &s_ge;
}
