#pragma once

#include <Eigen/Eigen>
#include <algorithm>
#include <iostream>
#include "Triangle.hpp"
#include "global.hpp"
using namespace Eigen;


namespace rst {
enum class Buffers { Color = 1, Depth = 2 };

inline Buffers operator|(Buffers a, Buffers b) {
  return Buffers((int)a | (int)b);
}

inline Buffers operator&(Buffers a, Buffers b) {
  return Buffers((int)a & (int)b);
}

enum class Primitive { Line, Triangle };

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

struct col_buf_id {
  int col_id = 0;
};


enum class blenmode { depth_test , src_alpha , one, None };

enum class CullMode { None, Back, Front };

class rasterizer {
 public:
  rasterizer(int w, int h);
  pos_buf_id load_positions(const std::vector<Eigen::Vector3f>& positions);
  ind_buf_id load_indices(const std::vector<Eigen::Vector3i>& indices);
  col_buf_id load_colors(const std::vector<Eigen::Vector4f>& colors);

  void set_alpha_blend_mode(int mode) {
      switch (mode) {
      case 0:
          blend_mode = blenmode::depth_test;
          break;
      case 1:
          blend_mode = blenmode::src_alpha;
          break;
      case 2:
          blend_mode = blenmode::one;
          break;
      case 3:
          blend_mode = blenmode::None;
          break;
      default:
          std::cerr << "Warning: Invalid blending mode " << mode << ". Keeping previous mode.\n";
          break;
      }
  }

  void set_cull_mode(int mode) {
      switch (mode) {
      case 0:
          cull_mode = CullMode::None;
          break;
      case 1:
          cull_mode = CullMode::Back;   // show front-facing only (cull back)
          break;
      case 2:
          cull_mode = CullMode::Front;  // show back-facing only (cull front)
          break;
      default:
          std::cerr << "Warning: Invalid cull mode " << mode << ". Keeping previous mode.\n";
          break;
      }
  }

  void set_model(const Eigen::Matrix4f& m);
  void set_view(const Eigen::Matrix4f& v);
  void set_projection(const Eigen::Matrix4f& p);

  void set_pixel(const Eigen::Vector3f& point, const Eigen::Vector4f& color);

  void clear(Buffers buff);

  void draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer,
            Primitive type);
  
  blenmode get_blend_mode() const { return blend_mode; }
  void set_blend_mode(blenmode mode) { blend_mode = mode; }

  CullMode get_cull_mode() const { return cull_mode; }
  void set_cull_mode(CullMode mode) { cull_mode = mode; }

  std::vector<Eigen::Vector4f>& frame_buffer() { return frame_buf; }

 private:
  void alpha_blend( Vector4f src, Vector4f& dst);
  void draw_line(Eigen::Vector3f begin, Eigen::Vector3f end);
  bool is_back_facing(const Eigen::Vector4f* v) const;

  void rasterize_triangle(const Triangle& t);

  // VERTEX SHADER -> MVP -> Clipping -> /.W -> VIEWPORT -> DRAWLINE/DRAWTRI ->
  // FRAGSHADER

 private:
  Eigen::Matrix4f model;
  Eigen::Matrix4f view;
  Eigen::Matrix4f projection;
  blenmode blend_mode = rst::blenmode::depth_test;
  CullMode cull_mode = CullMode::None;
  
  std::map<int, std::vector<Eigen::Vector3f>> pos_buf;
  std::map<int, std::vector<Eigen::Vector3i>> ind_buf;
  std::map<int, std::vector<Eigen::Vector4f>> col_buf;

  std::vector<Eigen::Vector4f> frame_buf;

  std::vector<float> depth_buf;
  int get_index(int x, int y);

  int width, height;

  int next_id = 0;
  int get_next_id() { return next_id++; }
};
}  // namespace rst
