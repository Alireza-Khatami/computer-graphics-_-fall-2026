#ifndef RASTERIZER_QUAD_H
#define RASTERIZER_QUAD_H

#include <Eigen/Eigen>

using namespace Eigen;

// A planar quadrilateral primitive, the four sided counterpart of Triangle.
//
// Using quads for a cube has a practical advantage over triangles: a cube is
// six quads instead of twelve triangles, and the wireframe shows only the
// twelve real edges of the cube - no diagonal across every face.
class Quad {
 public:
  Vector3f v[4]; /*the original coordinates of the quad, v0, v1, v2, v3 in
                   counter clockwise order*/
  /*Per vertex values*/
  Vector3f color[4];       // color at each vertex;
  Vector2f tex_coords[4];  // texture u,v
  Vector3f normal[4];      // normal vector for each vertex

  Quad();

  Eigen::Vector3f a() const { return v[0]; }
  Eigen::Vector3f b() const { return v[1]; }
  Eigen::Vector3f c() const { return v[2]; }
  Eigen::Vector3f d() const { return v[3]; }

  void setVertex(int ind, Vector3f ver); /*set i-th vertex coordinates */
  void setNormal(int ind, Vector3f n);   /*set i-th vertex normal vector*/
  void setColor(int ind, float r, float g, float b); /*set i-th vertex color*/
  void setTexCoord(int ind, float s,
                   float t); /*set i-th vertex texture coordinate*/
  std::array<Vector4f, 4> toVector4() const;
};

#endif  // RASTERIZER_QUAD_H
