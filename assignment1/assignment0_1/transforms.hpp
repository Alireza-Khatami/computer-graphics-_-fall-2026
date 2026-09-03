#pragma once

#include <Eigen/Eigen>

// Shared math helpers used by every scene.
constexpr double MY_PI = 3.1415926;
inline double DEG2RAD(double deg) { return deg * MY_PI / 180; }

// Simple view matrix: only translates the camera back to the origin.
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos);

// Model matrix: rotation around the Z axis (angle in degrees).
Eigen::Matrix4f get_model_matrix(float rotation_angle);

// Perspective projection matrix (M_ortho * M_persp->ortho).
Eigen::Matrix4f get_projection_matrix(float eye_fov,
                                      float aspect_ratio,
                                      float zNear,
                                      float zFar);

// View matrix built from a camera position, a look-at target and an up vector.
Eigen::Matrix4f get_view_matrix_lookat(Eigen::Vector3f eye,
                                       Eigen::Vector3f target,
                                       Eigen::Vector3f up);

// Model matrix composed from a uniform scale, a Z rotation and a translation.
Eigen::Matrix4f get_model_matrix_transformation(float scale,
                                                float rotation_angle,
                                                Eigen::Vector3f translation);
