#include "cube_scene.hpp"

#include <cmath>
#include <iostream>
#include <vector>

#include "transforms.hpp"

namespace {

// Where the cube sits in world space; the camera stays fixed and looks here.
const Eigen::Vector3f kCubeCenter(0, 0, -4);

// General LookAt view matrix: places the camera at `eye`, always facing
// `target`, with `up` used to resolve the camera's roll.
Eigen::Matrix4f lookat_view_matrix(Eigen::Vector3f eye, Eigen::Vector3f target, Eigen::Vector3f up) {
    Eigen::Vector3f z_axis = (eye - target).normalized();  // points from target to eye
    Eigen::Vector3f x_axis = up.cross(z_axis).normalized();
    Eigen::Vector3f y_axis = z_axis.cross(x_axis);

    Eigen::Matrix4f view;
    view << x_axis.x(), x_axis.y(), x_axis.z(), -x_axis.dot(eye),
            y_axis.x(), y_axis.y(), y_axis.z(), -y_axis.dot(eye),
            z_axis.x(), z_axis.y(), z_axis.z(), -z_axis.dot(eye),
            0,          0,          0,           1;
    return view;
}

// A point on a sphere of the given radius around `center`, used once to
// compute the cube scene's fixed camera position.
Eigen::Vector3f spherical_offset(Eigen::Vector3f center, float radius, float yaw_deg, float pitch_deg) {
    float yaw = yaw_deg * 3.14f / 180.0f;
    float pitch = pitch_deg * 3.14f / 180.0f;
    return center + Eigen::Vector3f(radius * cos(pitch) * sin(yaw),
                                     radius * sin(pitch),
                                     radius * cos(pitch) * cos(yaw));
}

// Rotates the cube around its own center: yaw_deg around Y (a/d), pitch_deg
// around X (w/s), then places it at kCubeCenter. Unlike a camera orbit, a
// plain rotation matrix has no gimbal singularity, so yaw/pitch are free to
// spin through a full circle with no clamp.
Eigen::Matrix4f cube_model_matrix(float yaw_deg, float pitch_deg) {
    float yaw = yaw_deg * 3.14f / 180.0f;
    float pitch = pitch_deg * 3.14f / 180.0f;
    Eigen::Matrix4f ry;
    ry << cos(yaw), 0, sin(yaw), 0,
          0,        1, 0,        0,
          -sin(yaw),0, cos(yaw), 0,
          0,        0, 0,        1;
    Eigen::Matrix4f rx;
    rx << 1, 0,          0,           0,
          0, cos(pitch), -sin(pitch), 0,
          0, sin(pitch), cos(pitch),  0,
          0, 0,          0,           1;
    Eigen::Matrix4f model = ry * rx;
    model(0, 3) = kCubeCenter.x();
    model(1, 3) = kCubeCenter.y();
    model(2, 3) = kCubeCenter.z();
    return model;
}

}  // namespace

// Builds a solid, opaque cube: 6 faces, 4 unique vertices per face (24
// total, no sharing across faces) so that each face can carry its own flat
// color, matching the "one color per triangle" style already used by the
// triangle scene. Faces are submitted nearest-of-each-pair-first:
// front/left/bottom (the three faces actually facing the camera at this
// rotation) are loaded before back/right/top (their hidden opposites). With
// correct z-buffering this submission order is irrelevant; without it, the
// later (hidden) faces paint over the visible ones wherever they overlap.
CubeScene::CubeScene(rst::rasterizer& r) {
    float s = 2.2f;
    std::vector<Eigen::Vector3f> pos = {
        // front (+z) - nearest of the front/back pair at this rotation
        {-s,-s, s}, { s,-s, s}, { s, s, s}, {-s, s, s},
        // left (-x) - nearest of the left/right pair
        {-s,-s,-s}, {-s,-s, s}, {-s, s, s}, {-s, s,-s},
        // bottom (-y) - nearest of the top/bottom pair
        {-s,-s,-s}, { s,-s,-s}, { s,-s, s}, {-s,-s, s},
        // back (-z) - hidden
        {-s,-s,-s}, { s,-s,-s}, { s, s,-s}, {-s, s,-s},
        // right (+x) - hidden
        { s,-s, s}, { s,-s,-s}, { s, s,-s}, { s, s, s},
        // top (+y) - hidden
        {-s, s, s}, { s, s, s}, { s, s,-s}, {-s, s,-s},
    };
    std::vector<Eigen::Vector3i> ind;
    for (int face = 0; face < 6; ++face) {
        int b = face * 4;
        ind.push_back({ b, b + 1, b + 2 });
        ind.push_back({ b, b + 2, b + 3 });
    }
    auto face_color = [](float r_, float g_, float b_) {
        return Eigen::Vector4f(r_, g_, b_, 1.0f);
    };
    std::vector<Eigen::Vector4f> face_colors = {
        face_color(220, 60, 60),   // front  - red
        face_color(60, 200, 120),  // left   - green
        face_color(60, 200, 200),  // bottom - cyan
        face_color(60, 120, 220),  // back   - blue
        face_color(230, 200, 60),  // right  - yellow
        face_color(200, 90, 200),  // top    - magenta
    };
    std::vector<Eigen::Vector4f> cols;
    for (auto& c : face_colors)
        for (int k = 0; k < 4; ++k)
            cols.push_back(c);

    pos_id_ = r.load_positions(pos);
    ind_id_ = r.load_indices(ind);
    col_id_ = r.load_colors(cols);

    cam_pos_ = spherical_offset(kCubeCenter, 14, 35, 20);
}

void CubeScene::render(rst::rasterizer& r) {
    render_with(r, blend_mode_);
}

void CubeScene::render_with(rst::rasterizer& r, int blend_mode) {
    r.set_cull_mode(0);  // None: let the z-buffer decide visibility
    r.set_alpha_blend_mode(blend_mode);
    r.set_model(cube_model_matrix(yaw_, pitch_));
    r.set_view(lookat_view_matrix(cam_pos_, kCubeCenter, { 0, 1, 0 }));
    r.set_projection(get_projection_matrix(90, 1, -0.1, -50));
    r.draw(pos_id_, ind_id_, col_id_, rst::Primitive::Triangle);
}

bool CubeScene::on_key(int key) {
    if (key == 'a') { yaw_ += 10; return true; }
    if (key == 'd') { yaw_ -= 10; return true; }
    if (key == 'w') { pitch_ += 10; return true; }
    if (key == 's') { pitch_ -= 10; return true; }
    if (key == 'b') { blend_mode_ = (blend_mode_ == 0) ? 1 : 0; return true; }
    return false;
}

void CubeScene::print_controls() const {
    std::cout << "\n-- Cube scene --\n"
              << "  a / d : spin cube left / right\n"
              << "  w / s : spin cube up / down\n"
              << "  b     : toggle blend mode ("
              << (blend_mode_ == 0 ? "depth_test" : "src_alpha")
              << ")\n"
              << "  TAB   : switch to triangle scene\n"
              << "  Esc   : quit\n";
}
