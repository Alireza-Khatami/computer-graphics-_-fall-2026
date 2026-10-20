#include "transforms.hpp"

#include <cmath>

Eigen::Matrix3f rotX(float a) {
    float c = std::cos(a), s = std::sin(a);
    Eigen::Matrix3f R; R <<
        1, 0, 0,
        0, c,-s,
        0, s, c;
    return R;
}

Eigen::Matrix3f rotY(float a) {
    float c = std::cos(a), s = std::sin(a);
    Eigen::Matrix3f R; R <<
         c, 0, s,
         0, 1, 0,
        -s, 0, c;
    return R;
}

Eigen::Matrix3f rotZ(float a) {
    float c = std::cos(a), s = std::sin(a);
    Eigen::Matrix3f R; R <<
        c,-s, 0,
        s, c, 0,
        0, 0, 1;
    return R;
}
