
#include "pch.h"

#undef min
#undef max
#include "cavc/polyline.hpp"

using namespace std;
using namespace geom;

using coord_t = double;

struct cShape
   : public iShape
   , public cavc::Polyline<coord_t>
{
   Type m_type;
   bool m_hole, m_filled;

   using poly = cavc::Polyline<coord_t>;

   cShape(Type type, bool hole, bool filled)
      : m_type(type)
      , m_hole(hole)
      , m_filled(filled)
   {
   }

   Type type() const override
   {
      return m_type;
   }

   bool empty() const override
   {
      return !poly::size();
   }
   bool closed() const override
   {
      return poly::isClosed();
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
   bool outline(iPolygon** res) const override
   {
      return false;
   }
   void holes(iPolygonIter** res) const override
   {
   }

   double length() const override
   {
      return cavc::getPathLength(*this);
   }
   double area() const override
   {
      return cavc::getArea(*this);
   }
   bool bounds(double& x1, double& y1, double& x2, double& y2) const override
   {
      if (empty()) {
         return false;
      }
      auto ext = cavc::getExtents(*this);
      tie(x1, y1, x2, y2) = { ext.xMin, ext.yMin, ext.xMax, ext.yMax };
      return true;
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
   int m_id = 0;
   string m_name;

   using ptr_type = unique_ptr<iShape>;

   struct cGeomTypeDesc
   {
      cavc::StaticSpatialIndex<coord_t> m_index;
      list<ptr_type> m_shapes_temp;
      vector<ptr_type> m_shapes;

      cGeomTypeDesc()
         : m_index(1)
      {
      }

      void add_shape(iShape* ps)
      {
         m_shapes_temp.push_back(ptr_type(ps));
      }

      void commit()
      {
         if (m_shapes_temp.size()) {
            m_shapes.clear();
            auto size = m_shapes_temp.size();
            m_shapes.reserve(size);
            cavc::StaticSpatialIndex<coord_t> index(size);
            for (auto& shape : m_shapes_temp) {
               double x1, y1, x2, y2;
               shape->bounds(x1, y1, x2, y2);
               index.add(x1, y1, x2, y2);
               m_shapes.push_back(move(shape));
            }
            index.finish();
            swap(m_index, index);
            m_shapes_temp.clear();
         }
      }

      struct cIter : public iShapeIter
      {
         vector<size_t> m_indices;
         vector<ptr_type>& m_shapes;
         size_t m_idx = -1;

         cIter(vector<ptr_type> & shapes)
            : m_shapes(shapes)
         {
         }
         void release() override
         {
            delete this;
         }
         bool first(iShape ** ps) override
         {
            m_idx = -1;
            return next(ps);
         }
         bool next(iShape ** ps) override
         {
            auto size = m_indices.size();
            if (size && (m_idx == -1 || m_idx < size - 1)) {
               *ps = m_shapes[++m_idx].get();
               return true;
            }
            return false;
         }
         size_t count() override
         {
            return m_indices.size();
         }
      };

      bool retrieve_shapes(iShapeIter** res, double xmin, double ymin, double xmax, double ymax, RetrieveOptions opt = RetrieveOptions::shape)
      {
         commit();
         vector<size_t> indices;
         m_index.query(xmin, ymin, xmax, ymax, indices);
         if (indices.size()) {
            auto pRes = new cIter(m_shapes);
            pRes->m_indices = move(indices);
            *res = pRes;
            return true;
         }
         *res = nullptr;
         return false;
      }
   };

   map<int, cGeomTypeDesc> m_shape_types;

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
      m_shape_types[type].add_shape(ps);
   }
   void remove_shape(iShape* ps) override
   {
   }
   bool retrieve_shapes(iShapeIter** res, double xmin, double ymin, double xmax, double ymax, int type, RetrieveOptions opt = RetrieveOptions::shape) const override
   {
      auto it = m_shape_types.find(type);
      if (it != m_shape_types.end()) {
         return const_cast<cGeomTypeDesc &>(it->second).retrieve_shapes(res, xmin, ymin, xmax, ymax, opt);
      }
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
      auto pShape = new cShape(iShape::Type::circle, hole, filled);
      pShape->addVertex(x - radius, y, 1);
      pShape->addVertex(x + radius, y, 1);
      pShape->isClosed() = true;
      *res = pShape;
   }
   void create_segment(iShape** res, double x1, double y1, double x2, double y2, double radius = 0, bool hole = false, bool filled = true) override
   {
   }
   void create_rectangle(iShape** res, double x1, double y1, double x2, double y2, bool hole = false, bool filled = true) override
   {
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

#include "gtest/gtest.h"

TEST(TestCaseName1, TestName1) {
   EXPECT_EQ(1, 1);
   EXPECT_TRUE(true);
}

class cTestFixture : public ::testing::Test
{
public:
   void SetUp() override
   {
      int i = 0;
   }
};

TEST_F(cTestFixture, test1) {
   EXPECT_EQ(1, 1);
}
