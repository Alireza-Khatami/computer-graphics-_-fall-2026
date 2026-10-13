#include "bezier.hpp"

Eigen::Vector3f de_casteljau(std::vector<Eigen::Vector3f> pts, float t) {
    int n = (int)pts.size();
    for (int r = 1; r < n; ++r)
        for (int i = 0; i < n - r; ++i)
            pts[i] = (1.0f - t) * pts[i] + t * pts[i + 1];
    return pts[0];
}

void precompute_lines(
    const std::vector<Eigen::Vector3f>& control_points_3d,
    int num_points_per_axis, int steps,
    std::vector<Eigen::Vector3f>& vertical,
    std::vector<Eigen::Vector3f>& horizontal,
    std::vector<Eigen::Vector3f>& diagonal)
{
    int n = num_points_per_axis;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n - 1; ++i) {
            Eigen::Vector3f p0 = control_points_3d[i * n + j];
            Eigen::Vector3f p1 = control_points_3d[(i + 1) * n + j];
            for (int k = 0; k <= steps; ++k) {
                float t = (float)k / steps;
                vertical.push_back((1.0f - t) * p0 + t * p1);
            }
        }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n - 1; ++j) {
            Eigen::Vector3f p0 = control_points_3d[i * n + j];
            Eigen::Vector3f p1 = control_points_3d[i * n + j + 1];
            for (int k = 0; k <= steps; ++k) {
                float t = (float)k / steps;
                horizontal.push_back((1.0f - t) * p0 + t * p1);
            }
        }
    for (int d = 0; d <= 2 * (n - 1); ++d) {
        std::vector<Eigen::Vector3f> diag;
        for (int i = 0; i < n; ++i) {
            int j = d - i;
            if (j >= 0 && j < n)
                diag.push_back(control_points_3d[i * n + j]);
        }
        if (diag.size() < 2) continue;
        for (size_t m = 0; m + 1 < diag.size(); ++m)
            for (int k = 0; k <= steps; ++k) {
                float t = (float)k / steps;
                diagonal.push_back((1.0f - t) * diag[m] + t * diag[m + 1]);
            }
    }
}

void precompute_bezier_curves(
    const std::vector<Eigen::Vector3f>& control_points_3d,
    int num_points_per_axis, int steps,
    std::vector<Eigen::Vector3f>& vertical,
    std::vector<Eigen::Vector3f>& horizontal,
    std::vector<Eigen::Vector3f>& diagonal)
{
    int n = num_points_per_axis;
    for (int j = 0; j < n; ++j) {
        std::vector<Eigen::Vector3f> col(n);
        for (int i = 0; i < n; ++i)
            col[i] = control_points_3d[i * n + j];
        for (int k = 0; k <= steps; ++k) {
            float t = (float)k / steps;
            vertical.push_back(de_casteljau(col, t));
        }
    }
    for (int i = 0; i < n; ++i) {
        std::vector<Eigen::Vector3f> row(n);
        for (int j = 0; j < n; ++j)
            row[j] = control_points_3d[i * n + j];
        for (int k = 0; k <= steps; ++k) {
            float t = (float)k / steps;
            horizontal.push_back(de_casteljau(row, t));
        }
    }
    for (int d = 0; d <= 2 * (n - 1); ++d) {
        std::vector<Eigen::Vector3f> diag;
        for (int i = 0; i < n; ++i) {
            int j = d - i;
            if (j >= 0 && j < n)
                diag.push_back(control_points_3d[i * n + j]);
        }
        if (diag.size() < 2) continue;
        for (int k = 0; k <= steps; ++k) {
            float t = (float)k / steps;
            diagonal.push_back(de_casteljau(diag, t));
        }
    }
}
