#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <future>
#include <thread>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>
// ============================================================
// Config
// ============================================================
static const int   WIDTH  = 900;
static const int   HEIGHT = 900;
static const float Z_NEAR = 1.0f;      // not used for clipping here, just reference
static const float CAM_Z  = 900.0f;    // camera distance (bigger = farther)
static const float FOCAL  = 900.0f;    // simple focal length for perspective
static const float STEP_T = 0.002f;    // curve sampling step
static const int   PT_RAD = 2;         // control point radius
std::vector<cv::Point3f> control_points_3d;
int num_points_per_axis ;  // default 4x4 control points
int grid_size ;
float initial_x_rotation = 0.0f;
float initial_y_rotation = 90.0f;
float initial_z_rotation = 0.0f;


// ============================================================
// Simple 3D transforms and perspective projection
// R = Rz(roll) * Ry(yaw) * Rx(pitch)
// Model is centered around its centroid, rotated, then translated +Z (CAM_Z)
// and projected with a pinhole camera model.
// ============================================================
Eigen::Matrix3f rotX(float a) {
    // TODO: Replace the Identity return with the correct 3x3 rotation matrix
    //       around the X axis by angle `a` (in radians).
    return Eigen::Matrix3f::Identity();

}
Eigen::Matrix3f rotY(float a) {
    // TODO: Replace the Identity return with the correct 3x3 rotation matrix
    //       around the Y axis by angle `a` (in radians).
    return Eigen::Matrix3f::Identity();


}
Eigen::Matrix3f rotZ(float a) {
    // TODO: Replace the Identity return with the correct 3x3 rotation matrix
    //       around the Z axis by angle `a` (in radians).
    return Eigen::Matrix3f::Identity();

}

Eigen::Vector3f de_casteljau(std::vector<Eigen::Vector3f> pts, float t) {
    // TODO: Implement the De Casteljau algorithm.
    //       Repeatedly reduce the point list by linearly interpolating each
    //       consecutive pair using parameter t, until one point remains.
    //       Return that final point.
    return pts[0];
}

void precompute_lines(
    const std::vector<Eigen::Vector3f>& control_points_3d,
    int num_points_per_axis, float step_t,
    std::vector<Eigen::Vector3f>& vertical,
    std::vector<Eigen::Vector3f>& horizontal,
    std::vector<Eigen::Vector3f>& diagonal)
{
    for (int j = 0; j < num_points_per_axis; ++j)
        for (int i = 0; i < num_points_per_axis - 1; ++i) {
            Eigen::Vector3f p0 = control_points_3d[i * num_points_per_axis + j];
            Eigen::Vector3f p1 = control_points_3d[(i+1) * num_points_per_axis + j];
            for (float t = 0.0f; t <= 1.0f; t += step_t)
                // TODO: Linearly interpolate between p0 and p1 at parameter t
                //       and push the result into `vertical`.

                {}
        }
    for (int i = 0; i < num_points_per_axis; ++i)
        for (int j = 0; j < num_points_per_axis - 1; ++j) {
            Eigen::Vector3f p0 = control_points_3d[i * num_points_per_axis + j];
            Eigen::Vector3f p1 = control_points_3d[i * num_points_per_axis + (j+1)];
            for (float t = 0.0f; t <= 1.0f; t += step_t)
                // TODO: Linearly interpolate between p0 and p1 at parameter t
                //       and push the result into `horizontal`.
                {}
            }
            int n = num_points_per_axis;
            for (int d = 0; d <= 2 * (n - 1); ++d) {
                std::vector<Eigen::Vector3f> diag;
                for (int i = 0; i < n; ++i) {
                    int j = d - i;
                    if (j >= 0 && j < n)
                    diag.push_back(control_points_3d[i * n + j]);
                }
                if (diag.size() < 2) continue;
                for (size_t k = 0; k < diag.size() - 1; ++k)
                for (float t = 0.0f; t <= 1.0f; t += step_t)
                // TODO: Linearly interpolate between diag[k] and diag[k+1] at parameter t
                //       and push the result into `diagonal`.
                
                {}
    }
}

void precompute_bezier_curves(
    const std::vector<Eigen::Vector3f>& control_points_3d,
    int num_points_per_axis, float step_t,
    std::vector<Eigen::Vector3f>& vertical,
    std::vector<Eigen::Vector3f>& horizontal,
    std::vector<Eigen::Vector3f>& diagonal)
{
    for (int j = 0; j < num_points_per_axis; ++j) {
        std::vector<Eigen::Vector3f> col(num_points_per_axis);
        for (int i = 0; i < num_points_per_axis; ++i)
            col[i] = control_points_3d[i * num_points_per_axis + j];
        for (float t = 0.0f; t <= 1.0f; t += step_t)
            // TODO: Evaluate the Bezier curve defined by control points `col` at parameter t
            //       using de_casteljau() and push the result into `vertical`.
            {}
    }
    for (int i = 0; i < num_points_per_axis; ++i) {
        std::vector<Eigen::Vector3f> row(num_points_per_axis);
        for (int j = 0; j < num_points_per_axis; ++j)
            row[j] = control_points_3d[i * num_points_per_axis + j];
        for (float t = 0.0f; t <= 1.0f; t += step_t)
            // TODO: Evaluate the Bezier curve defined by control points `row` at parameter t
            //       using de_casteljau() and push the result into `horizontal`.
            {}

    }
    int n = num_points_per_axis;
    for (int d = 0; d <= 2 * (n - 1); ++d) {
        std::vector<Eigen::Vector3f> diag;
        for (int i = 0; i < n; ++i) {
            int j = d - i;
            if (j >= 0 && j < n)
                diag.push_back(control_points_3d[i * n + j]);
        }
        if (diag.size() < 2) continue;
        for (float t = 0.0f; t <= 1.0f; t += step_t)
            // TODO: Evaluate the Bezier curve defined by the diagonal control points `diag` at parameter t
            //       using de_casteljau() and push the result into `diagonal`.
            {}
    }
}

Eigen::Vector2f project_point(
    const Eigen::Vector3f& p,
    const Eigen::Vector3f& centroid,
    float yaw, float pitch, float roll,
    float focal, float cam_z,
    int width, int height)
{
    Eigen::Matrix3f R = rotZ(roll) * rotY(yaw) * rotX(pitch);
    Eigen::Vector3f pc = p - centroid;        // center about centroid
    Eigen::Vector3f pr = R * pc;              // rotate
    pr.z() += cam_z;                           // push in front of camera

    // Perspective projection
    float x_ndc = (focal * pr.x()) / std::max(pr.z(), 1e-3f);
    float y_ndc = (focal * pr.y()) / std::max(pr.z(), 1e-3f);

    // Map to image pixels (cx, cy = image center)
    float cx = width  * 0.5f;
    float cy = height * 0.5f;
    return Eigen::Vector2f(cx + x_ndc, cy - y_ndc);
}

// ============================================================
// Compute centroid of 3D points (for rotation about center)
// ============================================================
Eigen::Vector3f compute_centroid(const std::vector<Eigen::Vector3f>& pts) {
    Eigen::Vector3f c(0,0,0);
    for (auto &p : pts) c += p;
    c /= static_cast<float>(pts.size());
    return c;
}




// Renders one snapshot of the scene to a cv::Mat using the given rotation (degrees) and mode
cv::Mat render_snapshot(
    const std::vector<Eigen::Vector3f>& control_points,
    int n_per_axis,
    float rx_deg, float ry_deg, float rz_deg,
    bool use_bezier)
{
    cv::Mat img(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));

    std::vector<Eigen::Vector3f> vert, horiz, diag;
    if (use_bezier)
        precompute_bezier_curves(control_points, n_per_axis, STEP_T, vert, horiz, diag);
    else
        precompute_lines(control_points, n_per_axis, STEP_T, vert, horiz, diag);

    Eigen::Vector3f centroid = compute_centroid(control_points);
    Eigen::Matrix3f R = rotZ(rz_deg * (float)M_PI / 180.0f)
                      * rotY(ry_deg * (float)M_PI / 180.0f)
                      * rotX(rx_deg * (float)M_PI / 180.0f);

    const float cx = WIDTH  * 0.5f;
    const float cy = HEIGHT * 0.5f;

    auto project = [&](const Eigen::Vector3f& p) -> cv::Point {
        Eigen::Vector3f v = R * (p - centroid);
        v.z() += CAM_Z;
        return { (int)std::lround(cx + (FOCAL * v.x()) / std::max(v.z(), 1e-3f)),
                 (int)std::lround(cy - (FOCAL * v.y()) / std::max(v.z(), 1e-3f)) };
    };

    for (const auto& p : control_points)
        cv::circle(img, project(p), PT_RAD, cv::Scalar(255, 255, 255), -1, cv::LINE_AA);

    for (const auto& p : vert) {
        cv::Point px = project(p);
        if (px.x >= 0 && px.x < WIDTH && px.y >= 0 && px.y < HEIGHT)
            img.at<cv::Vec3b>(px.y, px.x) = cv::Vec3b(255, 255, 0);
    }
    for (const auto& p : horiz) {
        cv::Point px = project(p);
        if (px.x >= 0 && px.x < WIDTH && px.y >= 0 && px.y < HEIGHT)
            img.at<cv::Vec3b>(px.y, px.x) = cv::Vec3b(255, 0, 255);
    }
    for (const auto& p : diag) {
        cv::Point px = project(p);
        if (px.x >= 0 && px.x < WIDTH && px.y >= 0 && px.y < HEIGHT)
            img.at<cv::Vec3b>(px.y, px.x) = cv::Vec3b(0, 255, 255);
    }

    return img;
}

int main(int argc, char** argv) {

    bool validate_mode = (argc == 2 && std::string(argv[1]) == "validate");

    bool is_curved = false;
    if (validate_mode) {
        num_points_per_axis = 10;
        grid_size = num_points_per_axis * num_points_per_axis;
    }
    else if (argc == 3){
        is_curved= argv[2];
        num_points_per_axis = std::max(2,  std::atoi(argv[1]));
    }
    else if (argc == 6){
        num_points_per_axis = std::max(2,  std::atoi(argv[2]));
        grid_size = num_points_per_axis * num_points_per_axis;
        //degree to radian
        std::string curve_mode = argv[1];
         if (curve_mode == "linear") {
            is_curved = false;
        } else if (curve_mode == "bezier") {
            is_curved = true;
        }
        else {
            std::cerr << "Invalid curve mode: " << curve_mode << std::endl;
            return -1;
        }
        initial_x_rotation = std::atof(argv[3]) * M_PI / 180.0f;
        initial_y_rotation = std::atof(argv[4]) * M_PI / 180.0f;
        initial_z_rotation = std::atof(argv[5]) * M_PI / 180.0f;
    }
    else {
        std::cerr << "Usage: " << argv[0] << " [num_control_points_per_axis] [curve_mode] [rotationX] [rotationY] [rotationZ]" << std::endl;
        return -1;
    }
    //create a 3D grid of control points
    std::vector<float> start_end_z = {-200.0f, 200.0f};
    std::vector<float> start_end_x = {200.0f, 500.0f};
    std::vector<float> start_end_y = {0.0f, 150.0f};
    std::vector<float> rowX;
    std::vector<float> rowZ;
    std::vector<float> rowY;
    int half = num_points_per_axis / 2;
    for (int i = 0; i < num_points_per_axis; ++i) {
        float z = start_end_z[0] + i * (start_end_z[1] - start_end_z[0]) / (num_points_per_axis - 1);
        float x = start_end_x[0] + i * (start_end_x[1] - start_end_x[0]) / (num_points_per_axis - 1);
        rowZ.push_back(z);
        rowX.push_back(x);
        float y ;
        // Y rises to 150 then falls back to 0
        if (i < half) {
            // 0 → 150
            float t_up = static_cast<float>(i) / half;
            y = start_end_y[0] + t_up * (start_end_y[1] - start_end_y[0]);
        } else {
            // 150 → 0
            float t_down = static_cast<float>(i - half) / (num_points_per_axis - half - 1);
            y = start_end_y[1] - t_down * (start_end_y[1] - start_end_y[0]);
        }
        rowY.push_back(y);
        }
    // Seed the same 16 control points (4x4 grid) as your 2D example
    std::vector<Eigen::Vector3f> control_points_3d;
    control_points_3d.reserve(num_points_per_axis*num_points_per_axis);
    for (int i = 0; i < num_points_per_axis; ++i) {
        for (int j = 0; j < num_points_per_axis; ++j) {
                int z_index = static_cast<int>((i+j*num_points_per_axis)/(num_points_per_axis));
                float x = rowX[i];
                float y = rowY[j] ;
                float z = rowZ[z_index];

                control_points_3d.emplace_back(x, y, z);
        }
    }
    for (int i=1; i<=num_points_per_axis-1; i++){
         control_points_3d[i].y() = 0;   // for edges
         control_points_3d[grid_size - i -1].y() =0;
    }

    if (validate_mode) {
        struct { float rx, ry, rz; std::string label; } configs[] = {
            { 90.f,  0.f,  0.f, "x90" },
            {  0.f, 90.f,  0.f, "y90" },
            {  0.f,  0.f, 0.0f, "z0" },
        };
        for (const auto& cfg : configs) {
            cv::imwrite("bezier_" + cfg.label + ".png",
                render_snapshot(control_points_3d, num_points_per_axis,
                                cfg.rx, cfg.ry, cfg.rz, true));
            cv::imwrite("linear_" + cfg.label + ".png",
                render_snapshot(control_points_3d, num_points_per_axis,
                                cfg.rx, cfg.ry, cfg.rz, false));
        }
        std::cout << "Saved 6 validation PNGs." << std::endl;
        return 0;
    }

    float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
    float cam_z = CAM_Z;
    const float dAng = 0.05f;  // radians per key press
    const float dCam = 25.0f;
    // rotation state matrix
    Eigen::Matrix3f R = Eigen::Matrix3f::Identity();
    //rotate initial_rotation around X to start with a better view
    R = rotZ(initial_z_rotation) * rotY(initial_y_rotation) * rotX(initial_x_rotation) * R;
    cv::Mat display(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
    bool dirty = true; // rerender only when rotation or camera changes

    Eigen::Vector3f centroid = compute_centroid(control_points_3d);

    // Precompute all curve sample points in object space (done once, never changes)
    std::vector<Eigen::Vector3f> cached_vertical_pts;
    std::vector<Eigen::Vector3f> cached_horizontal_pts;
    std::vector<Eigen::Vector3f> cached_diagonal_pts;

    if (!is_curved )
        precompute_lines(control_points_3d, num_points_per_axis, STEP_T,
                         cached_vertical_pts, cached_horizontal_pts, cached_diagonal_pts);
    else
        precompute_bezier_curves(control_points_3d, num_points_per_axis, STEP_T,
                                 cached_vertical_pts, cached_horizontal_pts, cached_diagonal_pts);

    // Precompute centered points once (avoids subtracting centroid every frame)
    auto center = [&](const std::vector<Eigen::Vector3f>& pts) {
        std::vector<Eigen::Vector3f> out(pts.size());
        for (size_t i = 0; i < pts.size(); ++i) out[i] = pts[i] - centroid;
        return out;
    };
    const std::vector<Eigen::Vector3f> ctr_ctrl     = center(control_points_3d);
    const std::vector<Eigen::Vector3f> ctr_vertical  = center(cached_vertical_pts);
    const std::vector<Eigen::Vector3f> ctr_horizontal= center(cached_horizontal_pts);
    const std::vector<Eigen::Vector3f> ctr_diagonal  = center(cached_diagonal_pts);

    // Cached projected pixel positions — only recomputed when R or cam_z changes
    std::vector<cv::Point> proj_ctrl, proj_vertical, proj_horizontal, proj_diagonal;
    const float cx = WIDTH  * 0.5f;
    const float cy = HEIGHT * 0.5f;

    const unsigned int nthreads = std::max(1u, std::thread::hardware_concurrency());

    // Splits a reproject call across multiple threads
    auto reproject = [&](const std::vector<Eigen::Vector3f>& centered, std::vector<cv::Point>& out) {
        out.resize(centered.size());
        size_t n = centered.size();
        size_t chunk = (n + nthreads - 1) / nthreads;
        std::vector<std::thread> threads;
        threads.reserve(nthreads);
        for (unsigned int t = 0; t < nthreads; ++t) {
            size_t start = t * chunk;
            size_t end   = std::min(start + chunk, n);
            if (start >= n) break;
            threads.emplace_back([&, start, end]() {
                for (size_t i = start; i < end; ++i) {
                    Eigen::Vector3f v = R * centered[i];
                    v.z() += cam_z;
                    out[i].x = (int)std::lround(cx + (FOCAL * v.x()) / std::max(v.z(), 1e-3f));
                    out[i].y = (int)std::lround(cy - (FOCAL * v.y()) / std::max(v.z(), 1e-3f));
                }
            });
        }
        for (auto& t : threads) t.join();
    };

    while (true) {
        int key = cv::waitKey(1);
        if (key == 27) break; // ESC
        if (key == 'a') { R = rotY(dAng) * R;   dirty = true; } // yaw left
        if (key == 'd') { R = rotY(-dAng) * R;  dirty = true; } // yaw right
        if (key == 'w') { R = rotX(dAng) * R;   dirty = true; } // pitch up
        if (key == 's') { R = rotX(-dAng) * R;  dirty = true; } // pitch down
        if (key == 'q') { R = rotZ(dAng) * R;   dirty = true; } // roll CCW
        if (key == 'e') { R = rotZ(-dAng) * R;  dirty = true; } // roll CW
        if (key == '+') { cam_z -= dCam;         dirty = true; } // zoom in
        if (key == '-') { cam_z += dCam;         dirty = true; } // zoom out

        if (dirty) {
            // Run all 4 reprojections concurrently — each writes to its own vector
            auto f1 = std::async(std::launch::async, [&]{ reproject(ctr_ctrl,       proj_ctrl); });
            auto f2 = std::async(std::launch::async, [&]{ reproject(ctr_vertical,   proj_vertical); });
            auto f3 = std::async(std::launch::async, [&]{ reproject(ctr_horizontal, proj_horizontal); });
            auto f4 = std::async(std::launch::async, [&]{ reproject(ctr_diagonal,   proj_diagonal); });
            f1.get(); f2.get(); f3.get(); f4.get();
            dirty = false;
        }

        display.setTo(cv::Scalar(0, 0, 0));

        for (const auto& p : proj_ctrl)
            cv::circle(display, p, PT_RAD, cv::Scalar(255,255,255), -1, cv::LINE_AA);
        for (const auto& p : proj_vertical)
            if (p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT)
                display.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(255, 255, 0);
        for (const auto& p : proj_horizontal)
            if (p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT)
                display.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(255, 0, 255);
        for (const auto& p : proj_diagonal)
            if (p.x >= 0 && p.x < WIDTH && p.y >= 0 && p.y < HEIGHT)
                display.at<cv::Vec3b>(p.y, p.x) = cv::Vec3b(0, 255, 255);

        cv::imshow("Bezier Surface 3D", display);
    }
    return 0;
}
