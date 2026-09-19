#pragma once

#include <Eigen/Eigen>

#include "rasterizer.hpp"
#include "scene.hpp"

// A solid, opaque cube deliberately misconfigured to skip the z-buffer: it
// renders with blend mode src_alpha instead of depth_test, so
// rasterize_triangle's depth-test branch never runs and faces simply paint
// over each other in submission order. Finding and fixing that
// misconfiguration - not any bug in rasterize_triangle/alpha_blend/culling -
// is the task; see the assignment writeup.
class CubeScene : public Scene {
 public:
  explicit CubeScene(rst::rasterizer& r);

  const char* name() const override { return "Cube"; }
  void render(rst::rasterizer& r) override;
  bool on_key(int key) override;
  void print_controls() const override;

  // Headless "cube" CLI mode: render once under an explicit blend mode
  // (0=depth_test/fixed, 1=src_alpha/buggy, 2=one), independent of whatever
  // the interactive toggle is currently set to.
  void render_with(rst::rasterizer& r, int blend_mode);

 private:
  rst::pos_buf_id pos_id_;
  rst::ind_buf_id ind_id_;
  rst::col_buf_id col_id_;

  Eigen::Vector3f cam_pos_;
  float yaw_ = 0.0f;
  float pitch_ = 0.0f;
  // Starts in the buggy configuration so the problem is visible immediately;
  // press b to toggle it.
  int blend_mode_ = 1;
};
