#pragma once

#include <eigen3/Eigen/Eigen>

// Camera / projection / model transforms shared by every scene.
// They are implemented in main.cpp - that is where get_projection_matrix()
// (a TODO for this assignment) lives.
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos);
Eigen::Vector3f orbit_eye_pos(float radius, float yaw_deg, float pitch_deg);
Eigen::Matrix4f get_model_matrix(float angle);
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar);
