#include "light.hpp"

namespace {

const std::vector<light> kDefaultWorldLights = {
    light{{20, 20, 20}, {500, 500, 500}},
    light{{-20, 20, 0}, {500, 500, 500}},
};

std::vector<light> g_world_lights = kDefaultWorldLights;
std::vector<light> g_view_space_lights = kDefaultWorldLights;

}  // namespace

const std::vector<light>& default_world_lights() { return kDefaultWorldLights; }

void set_world_lights(const std::vector<light>& lights) {
  g_world_lights = lights;
}

const std::vector<light>& world_lights() { return g_world_lights; }

Eigen::Vector3f ambient_light_intensity() { return {10, 10, 10}; }

void set_light_view_matrix(const Eigen::Matrix4f& view) {
  g_view_space_lights = g_world_lights;
  for (auto& l : g_view_space_lights) {
    Eigen::Vector4f p(l.position.x(), l.position.y(), l.position.z(), 1.0f);
    l.position = (view * p).head<3>();
  }
}

const std::vector<light>& view_space_lights() { return g_view_space_lights; }
