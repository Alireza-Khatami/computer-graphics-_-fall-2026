#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>

#include "transforms.hpp"

const int kWidth = 900;
const int kHeight = 900;

// Simple perspective view of a model. A point p is centered on the model's
// centroid, rotated by R, pushed cam_z in front of the camera and projected:
//   v = R (p - centroid) + (0, 0, cam_z)
//   x = cx + focal * v.x / v.z,   y = cy - focal * v.y / v.z
// R starts as Rz(rz) * Ry(ry) * Rx(rx); the keys then rotate the model
// around the screen axes.
struct View {
    Eigen::Matrix3f R = Eigen::Matrix3f::Identity();
    Eigen::Vector3f centroid = Eigen::Vector3f::Zero();
    float cam_z = 900.0f;  // camera distance (bigger = farther)
    float focal = 900.0f;  // focal length in pixels

    // Angles in degrees.
    void set_rotation(float rx_deg, float ry_deg, float rz_deg) {
        const float d2r = (float)M_PI / 180.0f;
        R = rotZ(rz_deg * d2r) * rotY(ry_deg * d2r) * rotX(rx_deg * d2r);
    }

    cv::Point project(const Eigen::Vector3f& p) const {
        Eigen::Vector3f v = R * (p - centroid);
        v.z() += cam_z;
        float z = std::max(v.z(), 1e-3f);
        return { (int)std::lround(kWidth * 0.5f + focal * v.x() / z),
                 (int)std::lround(kHeight * 0.5f - focal * v.y() / z) };
    }

    // W/S pitch, A/D yaw, Q/E roll, + / - zoom. Returns true if the view
    // changed.
    bool on_key(int key) {
        const float dAng = 0.05f;  // radians per key press
        const float dCam = 25.0f;
        switch (key) {
            case 'a': R = rotY(dAng) * R;  return true;  // yaw left
            case 'd': R = rotY(-dAng) * R; return true;  // yaw right
            case 'w': R = rotX(dAng) * R;  return true;  // pitch up
            case 's': R = rotX(-dAng) * R; return true;  // pitch down
            case 'q': R = rotZ(dAng) * R;  return true;  // roll CCW
            case 'e': R = rotZ(-dAng) * R; return true;  // roll CW
            case '+': case '=': cam_z -= dCam; return true;  // zoom in
            case '-': cam_z += dCam; return true;            // zoom out
        }
        return false;
    }

    // One pixel per sample (curves are sampled densely enough).
    void draw_samples(cv::Mat& img, const std::vector<Eigen::Vector3f>& pts,
                      const cv::Vec3b& color) const {
        for (const auto& p : pts) {
            cv::Point px = project(p);
            if (px.x >= 0 && px.x < img.cols && px.y >= 0 && px.y < img.rows)
                img.at<cv::Vec3b>(px.y, px.x) = color;
        }
    }

    void draw_control_points(cv::Mat& img,
                             const std::vector<Eigen::Vector3f>& pts) const {
        for (const auto& p : pts)
            cv::circle(img, project(p), 2, cv::Scalar(255, 255, 255), -1,
                       cv::LINE_AA);
    }
};

// Curve colors (BGR).
const cv::Vec3b kVerticalColor(255, 255, 0);    // cyan
const cv::Vec3b kHorizontalColor(255, 0, 255);  // magenta
const cv::Vec3b kDiagonalColor(0, 255, 255);    // yellow

inline Eigen::Vector3f compute_centroid(const std::vector<Eigen::Vector3f>& pts) {
    Eigen::Vector3f c(0, 0, 0);
    for (const auto& p : pts) c += p;
    return c / (float)pts.size();
}
