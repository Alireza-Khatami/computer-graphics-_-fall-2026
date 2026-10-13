#include "bezier.hpp"

Eigen::Vector3f de_casteljau(std::vector<Eigen::Vector3f> pts, float t) {
    // TODO: Implement the De Casteljau algorithm.
    //       Repeatedly reduce the point list by linearly interpolating each
    //       consecutive pair using parameter t, until one point remains.
    //       Return that final point.
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
                // TODO: Linearly interpolate between p0 and p1 at parameter t
                //       and push the result into `vertical`.
            }
        }
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n - 1; ++j) {
            Eigen::Vector3f p0 = control_points_3d[i * n + j];
            Eigen::Vector3f p1 = control_points_3d[i * n + j + 1];
            for (int k = 0; k <= steps; ++k) {
                float t = (float)k / steps;
                // TODO: Linearly interpolate between p0 and p1 at parameter t
                //       and push the result into `horizontal`.
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
                // TODO: Linearly interpolate between diag[m] and diag[m+1] at parameter t
                //       and push the result into `diagonal`.
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
            // TODO: Evaluate the Bezier curve defined by control points `col` at parameter t
            //       using de_casteljau() and push the result into `vertical`.
        }
    }
    for (int i = 0; i < n; ++i) {
        std::vector<Eigen::Vector3f> row(n);
        for (int j = 0; j < n; ++j)
            row[j] = control_points_3d[i * n + j];
        for (int k = 0; k <= steps; ++k) {
            float t = (float)k / steps;
            // TODO: Evaluate the Bezier curve defined by control points `row` at parameter t
            //       using de_casteljau() and push the result into `horizontal`.
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
            // TODO: Evaluate the Bezier curve defined by the diagonal control points `diag` at parameter t
            //       using de_casteljau() and push the result into `diagonal`.
        }
    }
}
