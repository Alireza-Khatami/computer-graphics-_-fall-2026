#pragma once

#include <Eigen/Eigen>

#include "scene.hpp"

// Scene 1: two triangles.
//   * the green triangle sits at the origin and is viewed through a look-at
//     camera that can orbit around it,
//   * the blue triangle is placed with a composed scale / rotate / translate
//     model matrix so the effect of the multiplication order can be observed.
class TriangleScene : public Scene {
 public:
  explicit TriangleScene(rst::rasterizer& r);

  const char* name() const override { return "Triangle"; }
  void render(rst::rasterizer& r) override;
  bool on_key(int key) override;
  void print_controls() const override;

  // Used by the headless (command line) rendering path.
  void set_first_triangle_angle(float angle) { angle_ = angle; }

 private:
  rst::pos_buf_id pos_id1_, pos_id2_;
  rst::ind_buf_id ind_id1_, ind_id2_;

  Eigen::Vector3f tri_1_color_{0.0f, 255.0f, 0.0f};
  Eigen::Vector3f tri_2_color_{0.0f, 0.0f, 255.0f};

  // Camera / first triangle.
  float angle_ = 0.0f;
  float eye_fov_ = 45.0f;
  float obj_eye_angle_ = 0.0f;

  // Second triangle transformation parameters.
  float tri2_scale_ = 1.0f;
  float tri2_rotation_ = 0.0f;
  float tri2_translate_x_ = 3.0f;
};
