#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "Texture.hpp"
#include "Triangle.hpp"
#include "camera.hpp"
#include "scene.hpp"

// The textured spot (cow) model lit by default_world_lights() and shaded with
// texture_fragment_shader (Blinn-Phong).
class SpotScene : public Scene {
 public:
  // An empty texture_path renders without a texture.
  SpotScene(const std::string& obj_path, const std::string& texture_path);

  const char* name() const override { return "Spot"; }
  void render(rst::rasterizer& r) override;
  bool on_key(int key) override { return camera_.on_key(key); }
  void print_controls() const override;

  // Defaults to texture_fragment_shader.
  void set_fragment_shader(
      std::function<Eigen::Vector3f(fragment_shader_payload)> shader) {
    shader_ = shader;
  }

 private:
  std::vector<Triangle*> triangles_;
  std::optional<Texture> texture_;
  float angle_ = 140.0f;  // model rotation around Y, degrees
  OrbitCamera camera_;
  std::function<Eigen::Vector3f(fragment_shader_payload)> shader_;
};
