#pragma once

#include "Object.hpp"
#include "Vector.hpp"

class Sphere : public Object {
 public:
  Sphere(const Vector3f& c, const float& r)
      : center(c), radius(r), radius2(r * r) {}

  // 'orig'  — ray origin
  // 'dir'   — ray direction (unit vector)
  // 'tnear' — output: distance to the nearest valid hit
  //
  // Returns true if the ray hits the sphere, false otherwise.
  bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear,
                 uint32_t&, Vector2f&) const override {
    // The ray-sphere intersection reduces to a quadratic equation in t,
    // where t is the distance along the ray to the hit point:
    //     P(t) = orig + t * dir
    //
    // Substituting into the sphere equation |P - center|² = radius² and
    // expanding gives:
    //     a·t² + b·t + c = 0
    // where:
    //     L = orig - center
    //     a = dir · dir
    //     b = 2 · (dir · L)
    //     c = (L · L) - radius²
    //
    // solveQuadratic solves this and returns the two roots t0 and t1.
    // It returns false if there is no real solution (ray misses the sphere).
    Vector3f L = orig - center;
    float a = dotProduct(dir, dir);
    float b = 2 * dotProduct(dir, L);
    float c = dotProduct(L, L) - radius2;
    float t0, t1;
    bool hasSolution = solveQuadratic(a, b, c, t0, t1);

    // TODO ---------------------------------------------------------------
    // t0 and t1 are the two candidate intersection distances along the ray.
    //     P(t) = orig + t * dir
    // A negative t means the intersection is *behind* the ray origin —
    // the sphere is behind the camera and should not be rendered.
    //
    // 1. If solveQuadratic found no real solution (hasSolution == false),
    //    the ray misses the sphere entirely → return false.
    //
    // 2. If t0 < 0, the nearer intersection is behind the origin.
    //    Try t1 instead (the ray may still hit the far side of the sphere).
    //
    // 3. If t1 < 0 as well, both intersections are behind the origin
    //    → return false.
    //
    // 4. Store the smallest positive t in tnear and return true.
    // --------------------------------------------------------------------

    return false;
  }

  void getSurfaceProperties(const Vector3f& P, const Vector3f&, const uint32_t&,
                            const Vector2f&, Vector3f& N,
                            Vector2f&) const override {
    N = normalize(P - center);
  }

  Vector3f center;
  float radius, radius2;
};
