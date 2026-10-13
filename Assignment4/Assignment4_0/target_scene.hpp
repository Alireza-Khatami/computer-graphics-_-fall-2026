#pragma once

#include <vector>

#include <Eigen/Eigen>

#include "scene.hpp"
#include "view.hpp"

// A 4 x 4 grid of control points P_ij (row i, column j) drawn with Bezier
// curves, plus a red target point T. The task: place the control point P_11
// (kP11 in target_scene.cpp) so that the Bezier curve of row 1,
// P_10 P_11 P_12 P_13, passes through T at t = 1/2. Work it out by hand;
// the point cannot be moved with the keyboard.
class TargetScene : public Scene {
 public:
    // Angles in degrees: the initial rotation of the view.
    TargetScene(float rx_deg = 0.0f, float ry_deg = 0.0f, float rz_deg = 0.0f);

    const char* name() const override { return "Target"; }
    void render(cv::Mat& image) const override;
    bool on_key(int key) override { return view_.on_key(key); }
    void print_controls() const override;

    // Prints P_11 solved from C(1/2) = T and checks it by evaluating the
    // curve with de_casteljau(). For the TA / reference solution.
    void print_solution() const;

 private:
    static const int n_ = 4;
    std::vector<Eigen::Vector3f> control_points_;  // row by row, P_ij at i * 4 + j
    std::vector<Eigen::Vector3f> vertical_, horizontal_, diagonal_;
    std::vector<Eigen::Vector3f> row1_;  // samples of the row 1 curve
    View view_;
};
