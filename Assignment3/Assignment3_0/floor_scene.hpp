#pragma once

#include <string>
#include <vector>

#include "Texture.hpp"
#include "Triangle.hpp"
#include "camera.hpp"
#include "scene.hpp"

// A textured floor with a red target circle and a single point light at a
// fixed height. The task: find the light's (x, z) that maximizes the
// radiance (Lambertian, so proportional to the irradiance
// E = I * cos(theta) / r^2) at the center of the circle, and set it in
// floor_scene.cpp (kLightX / kLightZ). The light cannot be moved with keys.
//
// The floor is shaded with diffuse_fragment_shader (no specular), so the
// answer does not depend on where the camera is.
class FloorScene : public Scene {
 public:
  // tilt_deg:    angle between the floor normal and +Y
  // azimuth_deg: horizontal direction the floor normal leans toward,
  //              measured from +X toward +Z
  // light_height: world-space y of the light (fixed)
  // intensity:   point light intensity I (same for r, g, b)
  FloorScene(const std::string& texture_path, float tilt_deg = 0.0f,
             float azimuth_deg = 30.0f, float light_height = 4.0f,
             float intensity = 16.0f);

  const char* name() const override { return "Floor"; }
  void render(rst::rasterizer& r) override;
  void annotate(cv::Mat& image) const override;
  bool on_key(int key) override;
  void print_controls() const override;

  Eigen::Vector3f light_position() const;

  // Irradiance at the center of the circle for the current light position:
  // E = I * max(0, n . l) / r^2 (the same diffuse term the shader uses).
  float irradiance_at_circle() const;

  // Prints the analytic optimum (x, z) and checks it with a brute-force
  // search over a grid of light positions. For the TA / reference solution.
  void print_solution() const;

 private:
  float irradiance_for(const Eigen::Vector3f& light_pos) const;

  std::vector<Triangle*> triangles_;
  Texture texture_;
  Eigen::Matrix4f model_;
  Eigen::Vector3f normal_;         // world-space floor normal
  Eigen::Vector3f circle_center_;  // world-space center of the circle
  float light_height_;
  float intensity_;
  OrbitCamera camera_;
  // Last frame's matrices, for annotate().
  Eigen::Matrix4f view_ = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f projection_ = Eigen::Matrix4f::Identity();
  int width_ = 700, height_ = 700;
};
