#pragma once

#include <vector>

#include <Eigen/Eigen>

// Number of segments each curve is sampled with: t = k / kCurveSteps for
// k = 0 .. kCurveSteps, so t = 0, t = 0.5 and t = 1 are sampled exactly.
const int kCurveSteps = 500;

// Point on the Bezier curve with control points `pts` at parameter t.
Eigen::Vector3f de_casteljau(std::vector<Eigen::Vector3f> pts, float t);

// The control points form an n x n grid stored row by row: point (i, j) is
// control_points_3d[i * n + j]. Both functions sample a curve along every
// column (vertical), every row (horizontal) and every anti-diagonal
// i + j = const (diagonal) and append the samples to the three containers.

// Straight line segments between neighbouring control points.
void precompute_lines(
    const std::vector<Eigen::Vector3f>& control_points_3d,
    int num_points_per_axis, int steps,
    std::vector<Eigen::Vector3f>& vertical,
    std::vector<Eigen::Vector3f>& horizontal,
    std::vector<Eigen::Vector3f>& diagonal);

// One Bezier curve per column, row and anti-diagonal.
void precompute_bezier_curves(
    const std::vector<Eigen::Vector3f>& control_points_3d,
    int num_points_per_axis, int steps,
    std::vector<Eigen::Vector3f>& vertical,
    std::vector<Eigen::Vector3f>& horizontal,
    std::vector<Eigen::Vector3f>& diagonal);
