#pragma once

#include "Object.hpp"
#include "Vector.hpp"
#include <cmath>

class Cylinder : public Object {
 public:
  Cylinder(const Vector3f& c, float r, float h)
      : center(c), radius(r), halfHeight(h * 0.5f), radius2(r * r) {}

  // index encoding: 0 = side surface, 1 = top cap, 2 = bottom cap
    bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear,
                  uint32_t& index, Vector2f&) const override {
    Vector3f oc = orig - center;

    float tBest = kInfinity;
    uint32_t iBest = 0;

    // Minimum t to avoid self-intersection (shadow acne).
    // Using 0 allows shadow rays to re-hit the originating surface at t~1e-7.
    const float tMin = 1e-4f;

    // --- Side surface (infinite cylinder along Y axis) ---
    // Skip when ray is nearly parallel to the axis: a = dir.x²+dir.z² ≈ 0
    // makes the quadratic ill-conditioned and produces garbage roots.
    float a = dir.x * dir.x + dir.z * dir.z;
    if (a > 1e-8f) {
      float b = 2.0f * (oc.x * dir.x + oc.z * dir.z);
      float c = oc.x * oc.x + oc.z * oc.z - radius2;

      float t0, t1;
      if (solveQuadratic(a, b, c, t0, t1)) {
        if (t0 > t1) std::swap(t0, t1);

        // Try nearer root first, then farther as fallback
        if (t0 > tMin) {
          float y = oc.y + t0 * dir.y;
          // Use inclusive bounds to avoid seam holes at cap edges
          if (y >= -halfHeight && y <= halfHeight) { tBest = t0; iBest = 0; }
        }
        if (t1 > tMin && t1 < tBest) {
          float y = oc.y + t1 * dir.y;
          if (y >= -halfHeight && y <= halfHeight) { tBest = t1; iBest = 0; }
        }
      }
    }

    // --- Cap intersections (ray-plane, then disk radius test) ---
    if (fabsf(dir.y) > 1e-8f) {
      // Top cap: solve  oc.y + t*dir.y = +halfHeight
      float tCap = (halfHeight - oc.y) / dir.y;
      if (tCap > tMin && tCap < tBest) {
        float px = oc.x + tCap * dir.x;
        float pz = oc.z + tCap * dir.z;
        if (px * px + pz * pz <= radius2) { tBest = tCap; iBest = 1; }
      }
      // Bottom cap: solve  oc.y + t*dir.y = -halfHeight
      tCap = (-halfHeight - oc.y) / dir.y;
      if (tCap > tMin && tCap < tBest) {
        float px = oc.x + tCap * dir.x;
        float pz = oc.z + tCap * dir.z;
        if (px * px + pz * pz <= radius2) { tBest = tCap; iBest = 2; }
      }
    }

    if (tBest == kInfinity) return false;

    tnear = tBest;
    index = iBest;
    return true;
  }

  void getSurfaceProperties(const Vector3f& P, const Vector3f&,
                            const uint32_t& index, const Vector2f&,
                            Vector3f& N, Vector2f& st) const override {
    if (index == 1) {
      // Top cap — flat upward normal, UV mapped to disk
      N = Vector3f(0, 1, 0);
      Vector3f lp = P - center;
      st.x = (lp.x / radius + 1.0f) * 0.5f;
      st.y = (lp.z / radius + 1.0f) * 0.5f;
    } else if (index == 2) {
      // Bottom cap — flat downward normal, UV mapped to disk
      N = Vector3f(0, -1, 0);
      Vector3f lp = P - center;
      st.x = (lp.x / radius + 1.0f) * 0.5f;
      st.y = (lp.z / radius + 1.0f) * 0.5f;
    } else {
      // Side surface — outward radial normal, cylindrical UV
      Vector3f lp = P - Vector3f(center.x, P.y, center.z);
      N = normalize(lp);
      // u: angle around the axis [0,1], v: height along axis [0,1]
      st.x = atan2f(lp.z, lp.x) / (2.0f * M_PI) + 0.5f;
      st.y = (P.y - (center.y - halfHeight)) / (2.0f * halfHeight);
    }
  }

  Vector3f evalDiffuseColor(const Vector2f& st) const override {
    float scale = 4.0f;
    float pattern =
        (fmodf(st.x * scale, 1.0f) > 0.5f) ^ (fmodf(st.y * scale, 1.0f) > 0.5f);
    return lerp(Vector3f(0.15f, 0.15f, 0.75f), Vector3f(1.0f, 1.0f, 1.0f), pattern);
  }

  Vector3f center;
  float radius;
  float halfHeight;
  float radius2;
};
