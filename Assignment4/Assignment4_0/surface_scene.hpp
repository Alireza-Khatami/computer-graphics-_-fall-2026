#pragma once

#include <vector>

#include <Eigen/Eigen>

#include "scene.hpp"
#include "view.hpp"

// An n x n grid of control points shaped like a ridge, drawn either with
// straight lines between neighbouring points (linear mode) or with one
// Bezier curve per row, column and anti-diagonal (Bezier mode).
class SurfaceScene : public Scene {
 public:
    // Angles in degrees: the initial rotation of the view.
    SurfaceScene(bool bezier, int num_points_per_axis,
                 float rx_deg, float ry_deg, float rz_deg);

    const char* name() const override { return "Surface"; }
    void render(cv::Mat& image) const override;
    bool on_key(int key) override { return view_.on_key(key); }
    void print_controls() const override;

 private:
    bool bezier_;
    int n_;
    std::vector<Eigen::Vector3f> control_points_;
    // Curve samples, computed once in the constructor.
    std::vector<Eigen::Vector3f> vertical_, horizontal_, diagonal_;
    View view_;
};
