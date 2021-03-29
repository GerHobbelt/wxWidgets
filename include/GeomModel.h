
#pragma once

#include "interface.h"

namespace geom {

   interface iPlane;
   interface iShape;
   interface iPolygon;

   interface iShapeIter
   {
      virtual void release() = 0;
      virtual bool first(iShape** ps) = 0;
      virtual bool next(iShape** ps) = 0;
      virtual size_t count() = 0;
   };
   interface iPolygonIter
   {
      virtual void release() = 0;
      virtual bool first(iPolygon** pc) = 0;
      virtual bool next(iPolygon** pc) = 0;
      virtual size_t count() = 0;
   };
   interface iVertexIter
   {
      virtual void release() = 0;
      virtual bool first(double& vertex_x, double& vertex_y) = 0;
      virtual bool next(double& vertex_x, double& vertex_y) = 0;
      virtual bool arc(double& center_x, double& center_y, double& radius) = 0;
      virtual size_t count() = 0;
   };

   interface iPolygon
   {
      enum class Type { unknown, circle, segment, rectangle, polyline };

      virtual Type type() const = 0;

      virtual bool empty() const = 0;
      virtual bool closed() const = 0;
      virtual bool filled() const = 0;
      virtual bool hole() const = 0;

      virtual double length() const = 0;
      virtual double area() const = 0;
      virtual bool bounds(double& x1, double& y1, double& x2, double& y2) const = 0;

      virtual void iter(iVertexIter** iter) = 0;

      virtual void bounding_box(iPolygon** res) const = 0;

      virtual bool equal(const iPolygon* ps, double tolerance = 0) const = 0;

      virtual bool includes(const iPolygon* ps, bool open_set = false) const = 0;
      virtual bool overlaps(const iPolygon* ps, bool open_set = false) const = 0;
      virtual bool touches(const iPolygon* ps) const = 0;

      virtual void add_vertex(double x, double y) = 0;
      virtual bool add_arc(double center_x, double center_y, double radius) = 0;
      virtual void commit() = 0;
   };

   interface iShape : public iPolygon
   {
      virtual bool outline(iPolygon** res) const = 0;
      virtual void holes(iPolygonIter** res) const = 0;

      virtual bool add_hole(const iShape* hole) = 0;
   };

   interface iTransform : public iShape
   {
      virtual void add_offset(double offset_x, double offset_y) = 0;
      virtual void add_rotation(double center_x, double center_y, double angle) = 0;
   };

   interface iPlane
   {
      enum class RetrieveOptions { shape, group, contour };

      virtual int id() const = 0;

      virtual void add_shape(iShape* ps, int type) = 0;
      virtual void remove_shape(iShape* ps) = 0;

      virtual bool retrieve_shapes(iShapeIter** res, double xmin, double ymin, double xmax, double ymax, int type, RetrieveOptions opt = RetrieveOptions::shape) const = 0;
   };

   interface iEngine
   {
      virtual void create_plane(iPlane** res, int id = 0, const char* name = 0) = 0;
      virtual bool plane(iPlane** res, const char* name) = 0;
      virtual bool plane(iPlane** res, int id) = 0;

      virtual void create_circle(iShape** res, double x, double y, double radius, bool hole = false, bool filled = true) = 0;
      virtual void create_segment(iShape** res, double x1, double y1, double x2, double y2, double radius = 0, bool hole = false, bool filled = true) = 0;
      virtual void create_rectangle(iShape** res, double x1, double y1, double x2, double y2, bool hole = false, bool filled = true) = 0;
      virtual void create_shape(iShape** res, bool hole = false, bool filled = true) = 0;
      virtual void create_shape(iShape** res, iShape* ps) = 0;

      virtual void expand(iShape** res, const iShape* ps, double dist) = 0;
      virtual void invert(iShape** res, const iShape* ps) = 0;

      virtual void intersect(iShape** res, const iShape* ps1, const iShape* ps2) = 0;
      virtual void merge(iShape** res, const iShape* ps1, const iShape* ps2) = 0;
      virtual void cut(iShape** res, const iShape* ps1, const iShape* ps2) = 0;

      virtual bool distance(const iShape* ps1, const iShape* ps2, double& distance, double* x1 = 0, double* y1 = 0, double* x2 = 0, double* y2 = 0) = 0;
   };

}; // namespace geom
