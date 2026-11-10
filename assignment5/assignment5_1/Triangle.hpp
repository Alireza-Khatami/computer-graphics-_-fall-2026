#pragma once

#include <cstring>

#include "Object.hpp"

// Möller–Trumbore ray-triangle intersection algorithm.
//
// 'v0', 'v1', 'v2' — the three vertices of the triangle (world space)
// 'orig'           — ray origin
// 'dir'            — ray direction (unit vector)
// 'tnear'          — output: distance along the ray to the hit point
// 'u', 'v'         — output: barycentric coordinates of the hit point
//
// Returns true if the ray hits the triangle, false otherwise.
//
// Background — barycentric coordinates:
//   Any point P inside a triangle can be written as:
//       P = (1 - u - v)·v0  +  u·v1  +  v·v2
//   where u >= 0, v >= 0, and u + v <= 1.
//   u and v are the outputs of this function and are later used by
//   getSurfaceProperties to interpolate texture coordinates across the triangle.
//
bool rayTriangleIntersect(const Vector3f& v0, const Vector3f& v1,
    const Vector3f& v2, const Vector3f& orig,
    const Vector3f& dir, float& tnear, float& u,
    float& v) {

    const float EPSILON = 1e-8f;

    // TODO ---------------------------------------------------------------
    // Implement the Möller–Trumbore algorithm.
    //
    // The key idea: a point on the ray and a point inside the triangle must
    // be the same hit point. Setting them equal gives a linear system solved
    // with Cramer's rule, broken into the steps below.
    //
    // Step 1 — compute the two edge vectors from v0:
    //     edge1 = v1 - v0
    //     edge2 = v2 - v0
    //
    // Step 2 — compute the determinant to check if the ray is parallel:
    //     pvec = crossProduct(dir, edge2)
    //     det  = dotProduct(edge1, pvec)
    //   If |det| < EPSILON the ray is parallel to the triangle → return false.
    //   Otherwise compute:
    //     invDet = 1.0f / det
    //
    // Step 3 — compute barycentric coordinate u:
    //     tvec = orig - v0
    //     u    = dotProduct(tvec, pvec) * invDet
    //   If u < 0 or u > 1 the hit is outside the triangle → return false.
    //
    // Step 4 — compute barycentric coordinate v:
    //     qvec = crossProduct(tvec, edge1)
    //     v    = dotProduct(dir, qvec) * invDet
    //   If v < 0 or (u + v) > 1 the hit is outside the triangle → return false.
    //
    // Step 5 — compute the distance t along the ray to the hit point:
    //     t = dotProduct(edge2, qvec) * invDet
    //   If t < EPSILON the intersection is behind the ray origin → return false.
    //
    // Step 6 — store the result and return:
    //     tnear = t
    //     return true
    // --------------------------------------------------------------------

    return false;
}

class MeshTriangle : public Object {
 public:
  MeshTriangle(const Vector3f* verts, const uint32_t* vertsIndex,
               const uint32_t& numTris, const Vector2f* st) {
    uint32_t maxIndex = 0;
    for (uint32_t i = 0; i < numTris * 3; ++i)
      if (vertsIndex[i] > maxIndex) maxIndex = vertsIndex[i];
    maxIndex += 1;
    vertices = std::unique_ptr<Vector3f[]>(new Vector3f[maxIndex]);
    memcpy(vertices.get(), verts, sizeof(Vector3f) * maxIndex);
    vertexIndex = std::unique_ptr<uint32_t[]>(new uint32_t[numTris * 3]);
    memcpy(vertexIndex.get(), vertsIndex, sizeof(uint32_t) * numTris * 3);
    numTriangles = numTris;
    stCoordinates = std::unique_ptr<Vector2f[]>(new Vector2f[maxIndex]);
    memcpy(stCoordinates.get(), st, sizeof(Vector2f) * maxIndex);
  }

  bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear,
                 uint32_t& index, Vector2f& uv) const override {
    bool intersect = false;
    for (uint32_t k = 0; k < numTriangles; ++k) {
      const Vector3f& v0 = vertices[vertexIndex[k * 3]];
      const Vector3f& v1 = vertices[vertexIndex[k * 3 + 1]];
      const Vector3f& v2 = vertices[vertexIndex[k * 3 + 2]];
      float t, u, v;
      if (rayTriangleIntersect(v0, v1, v2, orig, dir, t, u, v) && t < tnear) {
        tnear = t;
        uv.x = u;
        uv.y = v;
        index = k;
        intersect |= true;
      }
    }
    return intersect;
  }

  void getSurfaceProperties(const Vector3f&, const Vector3f&,
                            const uint32_t& index, const Vector2f& uv,
                            Vector3f& N, Vector2f& st) const override {
    const Vector3f& v0 = vertices[vertexIndex[index * 3]];
    const Vector3f& v1 = vertices[vertexIndex[index * 3 + 1]];
    const Vector3f& v2 = vertices[vertexIndex[index * 3 + 2]];
    Vector3f e0 = normalize(v1 - v0);
    Vector3f e1 = normalize(v2 - v1);
    N = normalize(crossProduct(e0, e1));
    const Vector2f& st0 = stCoordinates[vertexIndex[index * 3]];
    const Vector2f& st1 = stCoordinates[vertexIndex[index * 3 + 1]];
    const Vector2f& st2 = stCoordinates[vertexIndex[index * 3 + 2]];
    st = st0 * (1 - uv.x - uv.y) + st1 * uv.x + st2 * uv.y;
  }

  Vector3f evalDiffuseColor(const Vector2f& st) const override {
    float scale = 5;
    float pattern =
        (fmodf(st.x * scale, 1) > 0.5) ^ (fmodf(st.y * scale, 1) > 0.5);
    return lerp(Vector3f(0.815, 0.235, 0.031), Vector3f(0.937, 0.937, 0.231),
    // return lerp(Vector3f(0.215, 0.235, 0.031), Vector3f(0.937, 0.937, 0.531),
                pattern);
  }

  std::unique_ptr<Vector3f[]> vertices;
  uint32_t numTriangles;
  std::unique_ptr<uint32_t[]> vertexIndex;
  std::unique_ptr<Vector2f[]> stCoordinates;
};
