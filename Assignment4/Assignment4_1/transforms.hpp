#pragma once

#include <Eigen/Eigen>

// 3x3 rotation matrices around the X, Y and Z axes (angle in radians).
Eigen::Matrix3f rotX(float a);
Eigen::Matrix3f rotY(float a);
Eigen::Matrix3f rotZ(float a);
