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
float initial_y_rotation = 0.0f;
float initial_z_rotation = 0.0f;
// ============================================================
// Utility: clamp-safe plot a pixel 
// ============================================================
inline void draw_point(cv::Mat &img, const Eigen::Vector2f &p, const cv::Vec3b &color) {
    int x = std::clamp<int>(std::lround(p.x()), 0, img.cols - 1);
    int y = std::clamp<int>(std::lround(p.y()), 0, img.rows - 1);
    img.at<cv::Vec3b>(y, x) = color;
}

// ============================================================
// de Casteljau for Vector3f
// ============================================================
Eigen::Vector3f recursive_bezier_helper3D(const std::vector<Eigen::Vector3f>& pts, float t) {
    //TODO : implement de Casteljau algorithm for set of points pts , and time step t 
    return pts[0]; // placeholder
}

// ============================================================
// Simple 3D transforms and perspective projection
// R = Rz(roll) * Ry(yaw) * Rx(pitch)
// Model is centered around its centroid, rotated, then translated +Z (CAM_Z)
// and projected with a pinhole camera model.
// ============================================================
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


// ============================================================
// Render loop: draws control points, vertical & horizontal Bézier curves
// ============================================================
void render_scene_bezier(
    cv::Mat &frame,
    const std::vector<Eigen::Vector3f>& cp3d,
    float yaw, float pitch, float roll,
    float focal, float cam_z)
{
    frame.setTo(cv::Scalar(0,0,0));

    // Precompute centroid for stable rotations
    Eigen::Vector3f centroid = compute_centroid(cp3d);

    // Draw control points as small circles
    for (const auto &p3 : cp3d) {
        Eigen::Vector2f p2 = project_point(p3, centroid, yaw, pitch, roll, focal, cam_z, frame.cols, frame.rows);
        cv::circle(frame, cv::Point((int)std::lround(p2.x()), (int)std::lround(p2.y())), PT_RAD, cv::Scalar(255,255,255), -1, cv::LINE_AA);
    }

    // Colors
    const cv::Vec3b CYAN    = {255,255,0};  // BGR
    const cv::Vec3b MAGENTA = {255,0,255};

    // Draw vertical Bézier curves (u direction, fixed column j)
    for (int j = 0; j < num_points_per_axis; ++j) {
        for (float t = 0.0f; t <= 1.0f; t += STEP_T) {
            std::vector<Eigen::Vector3f> col(num_points_per_axis);
            for (int i = 0; i < num_points_per_axis; ++i)
                col[i] = cp3d[i * num_points_per_axis + j];
            Eigen::Vector3f p3 = recursive_bezier_helper3D(col, t);
            Eigen::Vector2f p2 = project_point(p3, centroid, yaw, pitch, roll, FOCAL, cam_z, frame.cols, frame.rows);
            draw_point(frame, p2, CYAN);
        }
    }

    // Draw horizontal Bézier curves (v direction, fixed row i)
    for (int i = 0; i < num_points_per_axis; ++i) {
        for (float t = 0.0f; t <= 1.0f; t += STEP_T) {
            std::vector<Eigen::Vector3f> row(num_points_per_axis);
            for (int j = 0; j < num_points_per_axis; ++j)
                row[j] = cp3d[i * num_points_per_axis + j];
            Eigen::Vector3f p3 = recursive_bezier_helper3D(row, t);
            Eigen::Vector2f p2 = project_point(p3, centroid, yaw, pitch, roll, FOCAL, cam_z, frame.cols, frame.rows);
            draw_point(frame, p2, MAGENTA);
        }
    }
    int n = num_points_per_axis;
    // Draw diagonal Bézier curves (v direction, fixed row i)
        for (int d = 0; d <= 2 * (n - 1); ++d) {
            std::vector<Eigen::Vector3f> diag;
            for (int i = 0; i < n; ++i) {
                int j = d - i;
                if (j >= 0 && j < n) {
                    diag.push_back(cp3d[i * n + j]);
                }
            }
        if (diag.size() <2) continue; // need at least 2 points to form a curve
        for (float t = 0.0f; t <= 1.0f; t += STEP_T) {
            Eigen::Vector3f p3 = recursive_bezier_helper3D(diag, t);
            Eigen::Vector2f p2 = project_point(p3, centroid, yaw, pitch, roll, FOCAL, cam_z, frame.cols, frame.rows);
            draw_point(frame, p2, cv::Vec3b(0,255,255)); // Yellow
        }
    }

}



// ============================================================
// Render loop: draws control points, vertical & horizontal Bézier curves
// ============================================================
void render_scene_normal(
    cv::Mat &frame,
    const std::vector<Eigen::Vector3f>& cp3d,
    float yaw, float pitch, float roll,
    float focal, float cam_z)
{
    frame.setTo(cv::Scalar(0,0,0));

    // Precompute centroid for stable rotations
    Eigen::Vector3f centroid = compute_centroid(cp3d);

    // Draw control points as small circles
    for (const auto &p3 : cp3d) {
        Eigen::Vector2f p2 = project_point(p3, centroid, yaw, pitch, roll, focal, cam_z, frame.cols, frame.rows);
        cv::circle(frame, cv::Point((int)std::lround(p2.x()), (int)std::lround(p2.y())), PT_RAD, cv::Scalar(255,255,255), -1, cv::LINE_AA);
    }

    // Colors
    const cv::Vec3b CYAN    = {255,255,0};  // BGR
    const cv::Vec3b MAGENTA = {255,0,255};

    // Draw vertical Bézier curves (u direction, fixed column j)
    for (int j = 0; j < num_points_per_axis; ++j) {
        for (int i = 0; i < num_points_per_axis-1; ++i){
            std::vector<Eigen::Vector3f> col(2);
            col[0] = cp3d[i * num_points_per_axis + j];
            col[1] = cp3d[(i+1) * num_points_per_axis + j];
            for (float t = 0.0f; t <= 1.0f; t += STEP_T) {
                Eigen::Vector3f p3 = t  * col[1] + (1.0f - t) * col[0];
                Eigen::Vector2f p2 = project_point(p3, centroid, yaw, pitch, roll, FOCAL, cam_z, frame.cols, frame.rows);
                draw_point(frame, p2, CYAN);
            }
        }
    }

    // Draw horizontal Bézier curves (v direction, fixed row i)
    for (int i = 0; i < num_points_per_axis; ++i) {
        for (int j = 0; j < num_points_per_axis-1; ++j) {
            std::vector<Eigen::Vector3f> row(2);
            row[0] = cp3d[i * num_points_per_axis + j];
            row[1] = cp3d[i * num_points_per_axis + (j+1)];
        for (float t = 0.0f; t <= 1.0f; t += STEP_T) {
            Eigen::Vector3f p3 = t  * row[1] + (1.0f - t) * row[0];
            Eigen::Vector2f p2 = project_point(p3, centroid, yaw, pitch, roll, FOCAL, cam_z, frame.cols, frame.rows);
            draw_point(frame, p2, MAGENTA);
            }
        }
    }

    int n = num_points_per_axis;
// Draw diagonal straight lines (instead of Bézier curves)
for (int d = 0; d <= 2 * (n - 1); ++d) {
    std::vector<Eigen::Vector3f> diag;
    for (int i = 0; i < n; ++i) {
        int j = d - i;
        if (j >= 0 && j < n) {
            diag.push_back(cp3d[i * n + j]);
        }
    }
    if (diag.size() < 2) continue; // need at least 2 points to draw a line

    // Draw straight lines between consecutive diagonal points
    for (size_t k = 0; k < diag.size() - 1; ++k) {
        Eigen::Vector3f p_start = diag[k];
        Eigen::Vector3f p_end = diag[k + 1];

        // Interpolate points along the line
        for (float t = 0.0f; t <= 1.0f; t += STEP_T) {
            Eigen::Vector3f p3 = p_start + t * (p_end - p_start);
            Eigen::Vector2f p2 = project_point(
                p3, centroid, yaw, pitch, roll, FOCAL, cam_z,
                frame.cols, frame.rows
            );
            draw_point(frame, p2, cv::Vec3b(0, 255, 255)); // Yellow
        }
    }c
}
}

int main(int argc, char** argv) {

    std::string is_curved =  "normal" ;
    if (argc == 3){
        is_curved= argv[2];
        num_points_per_axis = std::max(2,  std::atoi(argv[1]));
    }
    else if (argc == 5){
        num_points_per_axis = std::max(2,  std::atoi(argv[1])); 
        grid_size = num_points_per_axis * num_points_per_axis;
        //degree to radian
        initial_x_rotation = std::atof(argv[2]) * M_PI / 180.0f;
        initial_y_rotation = std::atof(argv[3]) * M_PI / 180.0f;
        initial_z_rotation = std::atof(argv[4]) * M_PI / 180.0f;
    }
    else {
        std::cerr << "Usage: " << argv[0] << " [num_control_points_per_axis] [rotationX] [rotationY] [rotationZ]" << std::endl;
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

    if (argc == 3) {
        struct RotCase {
            float rx, ry, rz;
            std::string name;
        };
        std::vector<RotCase> cases = {
            {0, 0, 90, "rotZ_90.jpg"},
            {90, 0, 0, "rotX_90.jpg"},
            {0, 90, 0, "rotY_90.jpg"}
        };

        for (const auto& c : cases) {
            Eigen::Matrix3f R = rotZ(c.rz * M_PI / 180.0f) *
                                rotY(c.ry * M_PI / 180.0f) *
                                rotX(c.rx * M_PI / 180.0f);

            std::vector<Eigen::Vector3f> rotated_pts = control_points_3d;
            Eigen::Vector3f centroid = compute_centroid(control_points_3d);
            for (auto& p : rotated_pts) {
                Eigen::Vector3f v = p - centroid;
                v = R * v;
                p = v + centroid;
            }

            cv::Mat frame(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(0, 0, 0));
            if (is_curved == "normal")
                render_scene_normal(frame, rotated_pts, 0, 0, 0, FOCAL, cam_z);
            else 
                render_scene_bezier(frame, rotated_pts, 0, 0, 0, FOCAL, cam_z);
            cv::imwrite(c.name, frame);
            std::cout << "Saved: " << c.name << std::endl;
        }
        return 0;
    }
    Eigen::Vector3f centroid = compute_centroid(control_points_3d);

    // Precompute all curve sample points in object space (done once, never changes)
    std::vector<Eigen::Vector3f> cached_vertical_pts;
    std::vector<Eigen::Vector3f> cached_horizontal_pts;
    std::vector<Eigen::Vector3f> cached_diagonal_pts;

    for (int j = 0; j < num_points_per_axis; ++j) {
        for (int i = 0; i < num_points_per_axis - 1; ++i) {
            Eigen::Vector3f p0 = control_points_3d[i * num_points_per_axis + j];
            Eigen::Vector3f p1 = control_points_3d[(i+1) * num_points_per_axis + j];
            for (float t = 0.0f; t <= 1.0f; t += STEP_T)
                cached_vertical_pts.push_back((1.0f - t) * p0 + t * p1);
        }
    }
    for (int i = 0; i < num_points_per_axis; ++i) {
        for (int j = 0; j < num_points_per_axis - 1; ++j) {
            Eigen::Vector3f p0 = control_points_3d[i * num_points_per_axis + j];
            Eigen::Vector3f p1 = control_points_3d[i * num_points_per_axis + (j+1)];
            for (float t = 0.0f; t <= 1.0f; t += STEP_T)
                cached_horizontal_pts.push_back((1.0f - t) * p0 + t * p1);
        }
    }
    {
        int n = num_points_per_axis;
        for (int d = 0; d <= 2 * (n - 1); ++d) {
            std::vector<Eigen::Vector3f> diag;
            for (int i = 0; i < n; ++i) {
                int j = d - i;
                if (j >= 0 && j < n)
                    diag.push_back(control_points_3d[i * n + j]);
            }
            if (diag.size() < 2) continue;
            for (size_t k = 0; k < diag.size() - 1; ++k) {
                for (float t = 0.0f; t <= 1.0f; t += STEP_T)
                    cached_diagonal_pts.push_back(diag[k] + t * (diag[k+1] - diag[k]));
            }
        }
    }

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

    auto reproject = [&](const std::vector<Eigen::Vector3f>& centered, std::vector<cv::Point>& out) {
        out.resize(centered.size());
        for (size_t i = 0; i < centered.size(); ++i) {
            Eigen::Vector3f v = R * centered[i];
            v.z() += cam_z;
            out[i].x = (int)std::lround(cx + (FOCAL * v.x()) / std::max(v.z(), 1e-3f));
            out[i].y = (int)std::lround(cy - (FOCAL * v.y()) / std::max(v.z(), 1e-3f));
        }
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
            reproject(ctr_ctrl,      proj_ctrl);
            reproject(ctr_vertical,  proj_vertical);
            reproject(ctr_horizontal,proj_horizontal);
            reproject(ctr_diagonal,  proj_diagonal);
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