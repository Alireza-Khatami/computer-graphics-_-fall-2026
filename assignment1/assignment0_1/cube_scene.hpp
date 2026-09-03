#pragma once

#include <Eigen/Eigen>

#include "scene.hpp"

// Scene 2: a wireframe cube viewed through a very wide angle lens.
//
// The cube is a much better probe for the projection matrix than a single
// triangle: a cube has parallel edges and square faces, so any mistake in the
// perspective / orthographic matrices (or a camera that is simply too close
// with too wide a field of view) is immediately visible as a distortion.
//
// The rasterizer clips triangles against the near plane, so the camera may be
// moved right up to - and even inside - the cube.
class CubeScene : public Scene {
 public:
  explicit CubeScene(rst::rasterizer& r);

  const char* name() const override { return "Cube"; }
  void render(rst::rasterizer& r) override;
  bool on_key(int key) override;
  void print_controls() const override;

  // Print the current camera setup and the framing invariant used by the
  // "dolly zoom" exercise.
  void print_camera() const;

 private:
  rst::pos_buf_id pos_id_;
  rst::ind_buf_id ind_id_;

  // NOTE: OpenCV interprets the frame buffer as BGR, so this is amber on screen.
  Eigen::Vector3f cube_color_{0.0f, 190.0f, 255.0f};

  // Camera. The defaults are deliberately aggressive: a very wide vertical
  // field of view with the eye close to the cube, which exaggerates the
  // perspective (strongly converging edges, front face much larger than the
  // back face).
  float eye_fov_ = 150.0f;
  float eye_dist_ = 2.5f;
  float orbit_ = 0.5f;   // rotation of the camera around the Y axis (radians)
  float eye_height_ = 0.8f;

  // Model transform of the cube itself.
  float spin_ = 0.0f;
};
