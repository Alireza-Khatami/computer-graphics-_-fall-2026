#include "target_scene.hpp"

#include <cstdio>
#include <iostream>

#include "bezier.hpp"

namespace {

// Control point grid: P_ij = (kX[j], kY[i][j], kZ[i]). P_11 is not taken
// from here, it is kP11 below.
const float kX[4] = {-150.0f, -50.0f, 50.0f, 150.0f};
const float kZ[4] = {-150.0f, -50.0f, 50.0f, 150.0f};
const float kY[4][4] = {{0, 0, 0, 0},
                        {0, 0, 60, 0},
                        {0, 60, 60, 0},
                        {0, 0, 0, 0}};

// The point the row 1 curve must pass through at t = 1/2.
const Eigen::Vector3f kTarget(-7.5f, 67.5f, -42.5f);

// TODO (target scene): set the control point P_11 so that the Bezier curve
// of row 1 (P_10 P_11 P_12 P_13) passes through kTarget at t = 1/2. Work
// it out by hand (see the handout) - the point cannot be moved with the
// keyboard.
const Eigen::Vector3f kP11(-50.0f, 60.0f, -50.0f);

// The view looks at the middle of the grid (fixed, so it does not move
// when P_11 changes).
const Eigen::Vector3f kViewCenter(0.0f, 40.0f, 0.0f);

const cv::Scalar kTargetColor(0, 0, 255);  // red
const cv::Scalar kRow1Color(0, 165, 255);  // orange

}  // namespace

TargetScene::TargetScene(float rx_deg, float ry_deg, float rz_deg) {
    for (int i = 0; i < n_; ++i)
        for (int j = 0; j < n_; ++j)
            control_points_.emplace_back(kX[j], kY[i][j], kZ[i]);
    control_points_[1 * n_ + 1] = kP11;

    precompute_bezier_curves(control_points_, n_, kCurveSteps,
                             vertical_, horizontal_, diagonal_);
    std::vector<Eigen::Vector3f> row1(control_points_.begin() + n_,
                                      control_points_.begin() + 2 * n_);
    for (int k = 0; k <= kCurveSteps; ++k)
        row1_.push_back(de_casteljau(row1, (float)k / kCurveSteps));

    view_.centroid = kViewCenter;
    view_.cam_z = 600.0f;
    view_.set_rotation(rx_deg, ry_deg, rz_deg);
}

void TargetScene::render(cv::Mat& image) const {
    view_.draw_samples(image, vertical_, kVerticalColor);
    view_.draw_samples(image, horizontal_, kHorizontalColor);
    view_.draw_samples(image, diagonal_, kDiagonalColor);
    // The row 1 curve on top, thicker.
    for (const auto& p : row1_)
        cv::circle(image, view_.project(p), 1, kRow1Color, -1);
    view_.draw_control_points(image, control_points_);

    // Label the row 1 control points and the target.
    for (int j = 0; j < n_; ++j) {
        cv::Point p = view_.project(control_points_[n_ + j]);
        cv::putText(image, "P1" + std::to_string(j), p + cv::Point(6, -6),
                    cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(255, 255, 255),
                    1, cv::LINE_AA);
    }
    cv::Point pp = view_.project(control_points_[n_ + 1]);
    cv::circle(image, pp, 5, kRow1Color, -1, cv::LINE_AA);
    cv::Point pt = view_.project(kTarget);
    cv::circle(image, pt, 8, kTargetColor, 2, cv::LINE_AA);
    cv::putText(image, "T", pt + cv::Point(10, 14), cv::FONT_HERSHEY_SIMPLEX,
                0.5, kTargetColor, 1, cv::LINE_AA);

    char buf[128];
    std::snprintf(buf, sizeof(buf), "T   = (%.2f, %.2f, %.2f)",
                  kTarget.x(), kTarget.y(), kTarget.z());
    cv::putText(image, buf, {10, 25}, cv::FONT_HERSHEY_SIMPLEX, 0.6,
                cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    std::snprintf(buf, sizeof(buf), "P11 = (%.2f, %.2f, %.2f)",
                  kP11.x(), kP11.y(), kP11.z());
    cv::putText(image, buf, {10, 50}, cv::FONT_HERSHEY_SIMPLEX, 0.6,
                cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
}

void TargetScene::print_controls() const {
    std::cout << "\n-- Target scene --\n"
              << "  w / s : pitch up / down\n"
              << "  a / d : yaw left / right\n"
              << "  q / e : roll counterclockwise / clockwise\n"
              << "  + / - : zoom in / out\n"
              << "  TAB   : switch scene\n"
              << "  Esc   : quit\n";
}
