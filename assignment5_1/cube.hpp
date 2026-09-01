#pragma once

#include "Object.hpp"
#include "Vector.hpp"
#include <algorithm>

class Cube : public Object {
 public:
  Cube(const Vector3f& minCorner, const Vector3f& maxCorner)
      : boundsMin(minCorner), boundsMax(maxCorner) {}

  bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear,
                 uint32_t&, Vector2f&) const override {
    // Ray-box intersection using the slabs method
    float tmin = (boundsMin.x - orig.x) / dir.x;
    float tmax = (boundsMax.x - orig.x) / dir.x;
    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (boundsMin.y - orig.y) / dir.y;
    float tymax = (boundsMax.y - orig.y) / dir.y;
    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
      return false;

    if (tymin > tmin)
      tmin = tymin;
    if (tymax < tmax)
      tmax = tymax;

    float tzmin = (boundsMin.z - orig.z) / dir.z;
    float tzmax = (boundsMax.z - orig.z) / dir.z;
    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
      return false;

    if (tzmin > tmin)
      tmin = tzmin;
    if (tzmax < tmax)
      tmax = tzmax;

    if (tmax < 0)
      return false;

    tnear = tmin > 0 ? tmin : tmax;
    return true;
  }

  void getSurfaceProperties(const Vector3f& P, const Vector3f&, const uint32_t&,
                            const Vector2f&, Vector3f& N,
                            Vector2f&) const override {
    // Compute normal based on which face was hit
    const float eps = 1e-4f;
    if (fabs(P.x - boundsMin.x) < eps)
        N = Vector3f(-1, 0, 0); // left face
    else if (fabs(P.x - boundsMax.x) < eps)
        N = Vector3f(1, 0, 0);  // right face
    else if (fabs(P.y - boundsMin.y) < eps)
        N = Vector3f(0, -1, 0); // bottom face
    else if (fabs(P.y - boundsMax.y) < eps)
        N = Vector3f(0, 1, 0);  // top face
    else if (fabs(P.z - boundsMin.z) < eps)
        N = Vector3f(0, 0, -1); // back face
    else
        N = Vector3f(0, 0, 1);  // front face
  }

  Vector3f boundsMin;
  Vector3f boundsMax;
};
