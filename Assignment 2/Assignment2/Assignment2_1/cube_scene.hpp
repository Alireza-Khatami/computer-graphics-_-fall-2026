#pragma once

#include <Eigen/Eigen>

#include "rasterizer.hpp"
#include "scene.hpp"

// A solid, opaque cube. Something about how it's rendered is wrong: the
// cube does not look the way a solid cube should. Figuring out what - and
// fixing it - is your task. The fix is not in rasterize_triangle,
// insideTriangle, alpha_blend, or the culling code in draw(); see the
// assignment writeup.
class CubeScene : public Scene {
 public:
  explicit CubeScene(rst::rasterizer& r);

  const char* name() const override { return "Cube"; }
  void render(rst::rasterizer& r) override;
  bool on_key(int key) override;
  void print_controls() const override;

 private:
  rst::pos_buf_id pos_id_;
  rst::ind_buf_id ind_id_;
  rst::col_buf_id col_id_;

  Eigen::Vector3f cam_pos_;
  float yaw_ = 0.0f;
  float pitch_ = 0.0f;
};
