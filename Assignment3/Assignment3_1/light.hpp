#pragma once

#include <eigen3/Eigen/Eigen>
#include <vector>

struct light {
  Eigen::Vector3f position;
  Eigen::Vector3f intensity;
};

// The two point lights of the spot (cow) scene, in world space.
const std::vector<light>& default_world_lights();

// The lights currently used for shading, in world space. Each scene sets its
// own lights before drawing; until then they are default_world_lights().
void set_world_lights(const std::vector<light>& lights);
const std::vector<light>& world_lights();

// Intensity of the ambient light.
Eigen::Vector3f ambient_light_intensity();

// The fragment shader receives view-space positions and normals, so the
// lights must be in view space too. Call this whenever the view matrix or
// the lights change (once per frame); view_space_lights() then returns the
// world lights moved by that matrix. Until it is called, the view is the
// identity.
void set_light_view_matrix(const Eigen::Matrix4f& view);
const std::vector<light>& view_space_lights();
