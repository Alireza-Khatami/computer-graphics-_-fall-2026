#pragma once

#include <Eigen/Eigen>

#include "rasterizer.hpp"
#include "scene.hpp"

// The two-triangle scene: a fixed green/pink pair plus a front/back-facing
// pair used to demonstrate depth testing, alpha blending and back-face
// culling. Interactive mode always renders it in depth_test / no-culling
// mode (a/d just orbits the camera); the headless CLI and validate paths
// call render_with() directly to render under a specific configuration.
class TriangleScene : public Scene {
 public:
  explicit TriangleScene(rst::rasterizer& r);

  const char* name() const override { return "Triangle"; }
  void render(rst::rasterizer& r) override;
  bool on_key(int key) override;
  void print_controls() const override;

  // Renders once under an explicit blend/cull configuration, independent of
  // the interactive defaults above. blend_mode/cull_mode use the same
  // integer encoding as rst::rasterizer::set_alpha_blend_mode/set_cull_mode.
  void render_with(rst::rasterizer& r, int blend_mode, int cull_mode);

 private:
  rst::pos_buf_id pos_id_;
  rst::ind_buf_id ind_id_;
  rst::col_buf_id col_id_;

  Eigen::Vector3f eye_pos_{0, 0, 10};
  float angle_ = 0.0f;
};
