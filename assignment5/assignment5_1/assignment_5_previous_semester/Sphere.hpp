#pragma once

#include "Object.hpp"
#include "Vector.hpp"

class Sphere : public Object {
 public:
  Sphere(const Vector3f& c, const float& r)
      : center(c), radius(r), radius2(r * r) {}

  bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear,
                 uint32_t&, Vector2f&) const override {
    // analytic solution
    Vector3f L = orig - center;
    float a = dotProduct(dir, dir);
    float b = 2 * dotProduct(dir, L);
    float c = dotProduct(L, L) - radius2;
    float t0, t1;
    bool hasSolution = solveQuadratic(a, b, c, t0, t1);
    // TODO ---------------------------------------------------------------
    // We have found the two potential intersection distances (t0, t1)
    // from the quadratic equation of the ray–sphere intersection.
    // Your task is to determine which one represents the correct hit.
    //
    // 1. Remember:
    //    - t0 and t1 are the parameter values along the ray:
    //        P(t) = orig + t * dir
    //    - A *negative* t means the intersection lies *behind* the ray origin
    //      (so the ray does not actually hit the sphere in the forward direction).
    //
    // 2. If both t0 and t1 are negative, there is **no valid intersection**.
    //    → return false.
    //
    // 3. Otherwise:
    //    - Choose the *smallest positive* t value as the nearest intersection.
    //      (Hint: if t0 < 0, use t1 instead.)
    //
    // 4. Store that value in `tnear` so the caller knows how far along the ray
    //    the intersection occurred.
    //
    // 5. Finally, return true to indicate that a valid intersection was found.
    //
    // ---------------------------------------------------------------------
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
