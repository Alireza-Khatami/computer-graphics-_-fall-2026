#pragma once

#include <algorithm>
#include <eigen3/Eigen/Eigen>

#include "transforms.hpp"

// Camera on a sphere around the origin, always looking at it (same controls
// as Assignment 2): a / d orbit left / right, w / s orbit up / down.
struct OrbitCamera {
  float radius = 10.0f;
  float yaw = 0.0f;    // degrees, around the Y axis
  float pitch = 0.0f;  // degrees, up / down

  Eigen::Matrix4f view() const {
    return get_view_matrix(orbit_eye_pos(radius, yaw, pitch));
  }

  // Returns true if the key moved the camera.
  bool on_key(int key) {
    if (key == 'a') { yaw -= 10; return true; }
    if (key == 'd') { yaw += 10; return true; }
    // Stay short of the poles, where LookAt's up vector becomes degenerate.
    if (key == 'w') { pitch = std::min(pitch + 10, 80.0f); return true; }
    if (key == 's') { pitch = std::max(pitch - 10, -80.0f); return true; }
    return false;
  }
};
