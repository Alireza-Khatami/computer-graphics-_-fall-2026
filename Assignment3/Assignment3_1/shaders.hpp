#pragma once

#include <eigen3/Eigen/Eigen>

#include "Shader.hpp"

// Vertex shader: passes the position through unchanged.
Eigen::Vector3f vertex_shader(const vertex_shader_payload& payload);

// Colors each fragment by its (view-space) normal; useful for debugging
// normal interpolation.
Eigen::Vector3f normal_fragment_shader(const fragment_shader_payload& payload);

// Blinn-Phong shading with the texture color (bilinear sampled) as kd.
// Lights come from view_space_lights() in light.hpp.
Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload);

// Lambertian (ambient + diffuse, no specular) shading with the texture color
// as kd. Without the view-dependent specular term, the outgoing radiance is
// proportional to the irradiance and looks the same from every camera
// position. Used by the floor scene.
Eigen::Vector3f diffuse_fragment_shader(const fragment_shader_payload& payload);
