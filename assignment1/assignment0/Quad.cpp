#include "Quad.hpp"

#include <algorithm>
#include <array>
#include <stdexcept>

Quad::Quad() {
  v[0] << 0, 0, 0;
  v[1] << 0, 0, 0;
  v[2] << 0, 0, 0;
  v[3] << 0, 0, 0;

  color[0] << 0.0, 0.0, 0.0;
  color[1] << 0.0, 0.0, 0.0;
  color[2] << 0.0, 0.0, 0.0;
  color[3] << 0.0, 0.0, 0.0;

  tex_coords[0] << 0.0, 0.0;
  tex_coords[1] << 0.0, 0.0;
  tex_coords[2] << 0.0, 0.0;
  tex_coords[3] << 0.0, 0.0;
}

void Quad::setVertex(int ind, Eigen::Vector3f ver) { v[ind] = ver; }

void Quad::setNormal(int ind, Vector3f n) { normal[ind] = n; }

void Quad::setColor(int ind, float r, float g, float b) {
  if ((r < 0.0) || (r > 255.) || (g < 0.0) || (g > 255.) || (b < 0.0) ||
      (b > 255.)) {
    throw std::runtime_error("Invalid color values");
  }

  color[ind] = Vector3f((float)r / 255., (float)g / 255., (float)b / 255.);
  return;
}

void Quad::setTexCoord(int ind, float s, float t) {
  tex_coords[ind] = Vector2f(s, t);
}

std::array<Vector4f, 4> Quad::toVector4() const {
  std::array<Vector4f, 4> res;
  std::transform(std::begin(v), std::end(v), res.begin(), [](auto& vec) {
    return Vector4f(vec.x(), vec.y(), vec.z(), 1.f);
  });
  return res;
}
