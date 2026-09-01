#pragma once

#include "Object.hpp"
#include "Vector.hpp"
#include <cmath>

class Cylinder : public Object {
 public:
  Cylinder(const Vector3f& c, float r, float h)
      : center(c), radius(r), halfHeight(h * 0.5f), radius2(r * r) {}

  // index encoding: 0 = side surface, 1 = top cap, 2 = bottom cap
  //
  // 'orig'  — ray origin (world space)
  // 'dir'   — ray direction (unit vector)
  // 'tnear' — output: distance to the nearest valid hit
  // 'index' — output: which surface was hit (0 = side, 1 = top cap, 2 = bottom cap)
  //
  // Returns true if the ray hits the cylinder, false otherwise.
  bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear,
                 uint32_t& index, Vector2f&) const override {

    // Translate the ray origin into the cylinder's local space so we can treat
    // the cylinder as if it were centered at the world origin.
    Vector3f oc = orig - center;

    // tBest tracks the closest valid hit found so far.
    // iBest tracks which surface that hit belongs to.
    float tBest = kInfinity;
    uint32_t iBest = 0;

    // Use tMin as the minimum accepted t value.
    // This prevents a surface from casting a shadow ray that immediately
    // re-intersects itself at t ≈ 0 (shadow acne).
    const float tMin = 1e-4f;

    // -----------------------------------------------------------------------
    // TODO — PART A: Side surface
    //
    // The cylinder is axis-aligned along Y. Ignoring the Y component, the
    // cross-section is a circle of radius 'radius' in the XZ plane:
    //
    //     x² + z² = radius²
    //
    // Substituting the ray  P(t) = oc + t * dir  gives a quadratic in t:
    //
    //     a·t² + b·t + c = 0
    //
    // where:
    //     a = dir.x² + dir.z²
    //     b = 2·(oc.x·dir.x + oc.z·dir.z)
    //     c = oc.x² + oc.z² − radius²
    //
    // 1. Compute a, b, c as above.
    // 2. Only proceed if a > 1e-8f  (skip near-vertical rays where a ≈ 0,
    //    as the quadratic becomes ill-conditioned and produces garbage roots).
    // 3. Call solveQuadratic(a, b, c, t0, t1) to get the two roots.
    // 4. Ensure t0 <= t1 (swap if needed).
    // 5. For each root (try t0 first, then t1 as fallback):
    //      - Reject if t <= tMin.
    //      - Reject if t >= tBest (a closer hit was already found).
    //      - Compute the Y position of the hit:  y = oc.y + t * dir.y
    //      - Accept only if  -halfHeight <= y <= halfHeight
    //        (the hit is on the finite wall, not the infinite extension).
    //      - If accepted: set tBest = t, iBest = 0.
    // -----------------------------------------------------------------------


    // -----------------------------------------------------------------------
    // TODO — PART B: Cap intersections
    //
    // Each cap is a flat disk on a horizontal plane:
    //   Top cap:    y = +halfHeight
    //   Bottom cap: y = -halfHeight
    //
    // Step 1 — find where the ray crosses the cap's plane.
    //   The ray reaches height H when:
    //       oc.y + t * dir.y = H   →   t = (H - oc.y) / dir.y
    //
    // Step 2 — check the hit point is inside the disk radius:
    //       px = oc.x + t * dir.x
    //       pz = oc.z + t * dir.z
    //       accept if  px² + pz² <= radius²
    //
    // 1. Only proceed if fabsf(dir.y) > 1e-8f  (skip rays parallel to the
    //    caps — they never cross the horizontal plane).
    // 2. For the TOP cap (H = +halfHeight):
    //      - Compute tCap = (+halfHeight - oc.y) / dir.y
    //      - Reject if tCap <= tMin or tCap >= tBest.
    //      - Compute px, pz at tCap and test  px² + pz² <= radius².
    //      - If accepted: set tBest = tCap, iBest = 1.
    // 3. Repeat for the BOTTOM cap (H = -halfHeight), setting iBest = 2.
    // -----------------------------------------------------------------------


    // If no surface was hit, report a miss.
    if (tBest == kInfinity) return false;

    // Report the closest hit.
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
