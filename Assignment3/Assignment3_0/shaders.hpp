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
