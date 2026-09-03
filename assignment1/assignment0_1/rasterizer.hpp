#pragma once

#include <Eigen/Eigen>
#include <algorithm>

#include "Quad.hpp"
#include "Triangle.hpp"
using namespace Eigen;

namespace rst {
enum class Buffers { Color = 1, Depth = 2 };

inline Buffers operator|(Buffers a, Buffers b) {
  return Buffers((int)a | (int)b);
}

inline Buffers operator&(Buffers a, Buffers b) {
  return Buffers((int)a & (int)b);
}

enum class Primitive { Line, Triangle, Quad };

/*
 * For the curious : The draw function takes two buffer id's as its arguments.
 * These two structs make sure that if you mix up with their orders, the
 * compiler won't compile it. Aka : Type safety
 * */
struct pos_buf_id {
  int pos_id = 0;
};

struct ind_buf_id {
  int ind_id = 0;
};

class rasterizer {
 public:
  rasterizer(int w, int h);
  pos_buf_id load_positions(const std::vector<Eigen::Vector3f>& positions);
  ind_buf_id load_indices(const std::vector<Eigen::Vector3i>& indices);
  // Overload for quad meshes: every entry lists the four corners of one face.
  ind_buf_id load_indices(const std::vector<Eigen::Vector4i>& indices);

  void set_model(const Eigen::Matrix4f& m);
  void set_view(const Eigen::Matrix4f& v);
  void set_projection(const Eigen::Matrix4f& p);

  // Z of the clip plane in view space. Primitives are clipped against it
  // before the perspective divide. The default is a tiny negative epsilon,
  // i.e. the plane of the camera itself, so that geometry stays visible right
  // up to the eye. It cannot be exactly 0: the perspective divide is a
  // division by w = z_view, and z_view = 0 projects to infinity.
  void set_near(float z_near);

  void set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color);

  void clear(Buffers buff);

  void draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, Primitive type, Eigen::Vector3f& color );

  std::vector<Eigen::Vector3f>& frame_buffer() { return frame_buf; }

  int get_width() const { return width; }
  int get_height() const { return height; }

 private:
  void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end, Eigen::Vector3f& color);
  void rasterize_wireframe(const Triangle& t, Eigen::Vector3f& color);
  void rasterize_wireframe(const Quad& q, Eigen::Vector3f& color);
  // Fallback for a polygon that is neither a triangle nor a quad, which is
  // what near plane clipping can produce. Draws the closed outline.
  void rasterize_wireframe(const std::vector<Eigen::Vector3f>& poly,
                           Eigen::Vector3f& color);

  // Model -> view -> clip against the near plane -> projection -> viewport.
  // Returns the screen space polygon, empty if it is fully clipped away.
  std::vector<Eigen::Vector3f> project_polygon(
      const std::vector<Eigen::Vector3f>& object_space);

 private:
  Eigen::Matrix4f model;
  Eigen::Matrix4f view;
  Eigen::Matrix4f projection;
  float near_plane = -1e-4f;  // effectively the camera plane

  std::map<int, std::vector<Eigen::Vector3f>> pos_buf;
  std::map<int, std::vector<Eigen::Vector3i>> ind_buf;
  std::map<int, std::vector<Eigen::Vector4i>> quad_ind_buf;

  std::vector<Eigen::Vector3f> frame_buf;
  std::vector<float> depth_buf;
  int get_index(int x, int y);

  int width, height;

  int next_id = 0;
  int get_next_id() { return next_id++; }
};
}  // namespace rst
