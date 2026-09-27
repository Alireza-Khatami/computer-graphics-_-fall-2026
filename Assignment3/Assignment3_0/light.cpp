#include "light.hpp"

namespace {

const std::vector<light> kWorldLights = {
    light{{20, 20, 20}, {500, 500, 500}},
    light{{-20, 20, 0}, {500, 500, 500}},
};

std::vector<light> g_view_space_lights = kWorldLights;

}  // namespace

const std::vector<light>& world_lights() { return kWorldLights; }

Eigen::Vector3f ambient_light_intensity() { return {10, 10, 10}; }

void set_light_view_matrix(const Eigen::Matrix4f& view) {
  g_view_space_lights = kWorldLights;
  for (auto& l : g_view_space_lights) {
    Eigen::Vector4f p(l.position.x(), l.position.y(), l.position.z(), 1.0f);
    l.position = (view * p).head<3>();
  }
}

const std::vector<light>& view_space_lights() { return g_view_space_lights; }
