#pragma once

#include <eigen3/Eigen/Eigen>
#include <vector>

struct light {
  Eigen::Vector3f position;
  Eigen::Vector3f intensity;
};

// The scene's point lights, fixed in world space.
const std::vector<light>& world_lights();

// Intensity of the ambient light.
Eigen::Vector3f ambient_light_intensity();

// The fragment shader receives view-space positions and normals, so the
// lights must be in view space too. Call this whenever the view matrix
// changes (once per frame); view_space_lights() then returns the lights
// moved by that matrix. Until it is called, the view is the identity.
void set_light_view_matrix(const Eigen::Matrix4f& view);
const std::vector<light>& view_space_lights();
