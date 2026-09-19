#pragma once

#include <Eigen/Eigen>

// Camera / projection / model transforms shared by every scene.
// get_model_matrix() and get_projection_matrix() are implemented in
// main.cpp - that is where the TODOs for this assignment live.
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos, float angle);
Eigen::Matrix4f get_model_matrix(float rotation_angle);
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
    float zNear, float zFar);
