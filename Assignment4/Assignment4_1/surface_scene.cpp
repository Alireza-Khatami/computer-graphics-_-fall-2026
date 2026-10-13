#include "surface_scene.hpp"

#include <iostream>

#include "bezier.hpp"

SurfaceScene::SurfaceScene(bool bezier, int num_points_per_axis,
                           float rx_deg, float ry_deg, float rz_deg)
    : bezier_(bezier), n_(std::max(2, num_points_per_axis)) {
    // Row i has x = rowX[i]; column j has height rowY[j], which rises to 150
    // in the middle and falls back to 0; z = rowZ[j].
    const float start_end_z[2] = {-200.0f, 200.0f};
    const float start_end_x[2] = {200.0f, 500.0f};
    const float start_end_y[2] = {0.0f, 150.0f};
    std::vector<float> rowX, rowY, rowZ;
    int half = n_ / 2;
    for (int i = 0; i < n_; ++i) {
        rowZ.push_back(start_end_z[0] + i * (start_end_z[1] - start_end_z[0]) / (n_ - 1));
        rowX.push_back(start_end_x[0] + i * (start_end_x[1] - start_end_x[0]) / (n_ - 1));
        float y;
        if (i < half) {
            // 0 -> 150
            float t_up = static_cast<float>(i) / half;
            y = start_end_y[0] + t_up * (start_end_y[1] - start_end_y[0]);
        } else {
            // 150 -> 0
            float t_down = static_cast<float>(i - half) / (n_ - half - 1);
            y = start_end_y[1] - t_down * (start_end_y[1] - start_end_y[0]);
        }
        rowY.push_back(y);
    }
    for (int i = 0; i < n_; ++i)
        for (int j = 0; j < n_; ++j)
            control_points_.emplace_back(rowX[i], rowY[j], rowZ[j]);
    // The first and last rows lie flat on y = 0 (the edges of the surface).
    int grid_size = n_ * n_;
    for (int i = 1; i <= n_ - 1; ++i) {
        control_points_[i].y() = 0;
        control_points_[grid_size - i - 1].y() = 0;
    }

    if (bezier_)
        precompute_bezier_curves(control_points_, n_, kCurveSteps,
                                 vertical_, horizontal_, diagonal_);
    else
        precompute_lines(control_points_, n_, kCurveSteps,
                         vertical_, horizontal_, diagonal_);

    view_.centroid = compute_centroid(control_points_);
    view_.set_rotation(rx_deg, ry_deg, rz_deg);
}

void SurfaceScene::render(cv::Mat& image) const {
    view_.draw_control_points(image, control_points_);
    view_.draw_samples(image, vertical_, kVerticalColor);
    view_.draw_samples(image, horizontal_, kHorizontalColor);
    view_.draw_samples(image, diagonal_, kDiagonalColor);
}

void SurfaceScene::print_controls() const {
    std::cout << "\n-- Surface scene (" << (bezier_ ? "bezier" : "linear")
              << ", " << n_ << " x " << n_ << " control points) --\n"
              << "  w / s : pitch up / down\n"
              << "  a / d : yaw left / right\n"
              << "  q / e : roll counterclockwise / clockwise\n"
              << "  + / - : zoom in / out\n"
              << "  TAB   : switch scene\n"
              << "  Esc   : quit\n";
}
