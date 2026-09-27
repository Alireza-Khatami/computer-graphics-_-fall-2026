#include "floor_scene.hpp"

#include <cmath>
#include <cstdio>
#include <iostream>

#include "global.hpp"
#include "light.hpp"
#include "shaders.hpp"
#include "transforms.hpp"

namespace {

const float kHalfSize = 4.0f;  // the floor spans [-4, 4] x [-4, 4] (local x, z)
const int kGrid = 16;          // quads per side

// Texture coordinates (u, v) of the center of the red circle painted on
// floor_texture.png. Texture coordinates run 0..1 across the floor:
// u = (x + 4) / 8 and v = (z + 4) / 8.
const float kCircleU = 0.6875f;
const float kCircleV = 0.375f;

// TODO (floor scene): set the light's world-space x and z so that the
// radiance at the center of the circle is as large as possible. Work the
// position out by hand (see the handout) - the light cannot be moved with
// the keyboard. Its height is fixed at light_height (y = 4).
const float kLightX = 0.0f;
const float kLightZ = 0.0f;

// Rotation that tilts +Y by tilt_deg toward the horizontal direction
// azimuth_deg (measured from +X toward +Z).
Eigen::Matrix3f tilt_rotation(float tilt_deg, float azimuth_deg) {
  float tilt = tilt_deg * MY_PI / 180.f;
  float azimuth = azimuth_deg * MY_PI / 180.f;
  Eigen::Vector3f n(std::sin(tilt) * std::cos(azimuth), std::cos(tilt),
                    std::sin(tilt) * std::sin(azimuth));
  Eigen::Vector3f axis = Eigen::Vector3f::UnitY().cross(n);
  if (axis.norm() < 1e-6f) return Eigen::Matrix3f::Identity();
  return Eigen::AngleAxisf(tilt, axis.normalized()).toRotationMatrix();
}

}  // namespace

FloorScene::FloorScene(const std::string& texture_path, float tilt_deg,
                       float azimuth_deg, float light_height, float intensity)
    : texture_(texture_path),
      light_height_(light_height),
      intensity_(intensity) {
  Eigen::Matrix3f rot = tilt_rotation(tilt_deg, azimuth_deg);
  model_ = Eigen::Matrix4f::Identity();
  model_.topLeftCorner<3, 3>() = rot;
  normal_ = rot * Eigen::Vector3f::UnitY();
  Eigen::Vector4f circle_local(-kHalfSize + 2 * kHalfSize * kCircleU, 0,
                               -kHalfSize + 2 * kHalfSize * kCircleV, 1);
  circle_center_ = (model_ * circle_local).head<3>();

  // A flat grid in local space (y = 0, normal +Y); the model matrix tilts it.
  // Texture coordinates run 0..1 across the floor (see kCircleU / kCircleV).
  auto vertex = [](int i, int j) {
    float x = -kHalfSize + 2 * kHalfSize * i / kGrid;
    float z = -kHalfSize + 2 * kHalfSize * j / kGrid;
    return Eigen::Vector3f(x, 0, z);
  };
  auto uv = [](const Eigen::Vector3f& p) {
    return Eigen::Vector2f((p.x() + kHalfSize) / (2 * kHalfSize),
                           (p.z() + kHalfSize) / (2 * kHalfSize));
  };
  auto add = [&](const Eigen::Vector3f& a, const Eigen::Vector3f& b,
                 const Eigen::Vector3f& c) {
    Triangle* t = new Triangle();
    const Eigen::Vector3f* v[3] = {&a, &b, &c};
    for (int k = 0; k < 3; ++k) {
      t->setVertex(k, Vector4f(v[k]->x(), v[k]->y(), v[k]->z(), 1.0f));
      t->setNormal(k, Vector3f(0, 1, 0));
      t->setTexCoord(k, uv(*v[k]));
    }
    triangles_.push_back(t);
  };
  for (int i = 0; i < kGrid; ++i) {
    for (int j = 0; j < kGrid; ++j) {
      Eigen::Vector3f p00 = vertex(i, j), p10 = vertex(i + 1, j);
      Eigen::Vector3f p01 = vertex(i, j + 1), p11 = vertex(i + 1, j + 1);
      add(p00, p01, p11);
      add(p00, p11, p10);
    }
  }

  camera_.radius = 12.0f;
  camera_.pitch = 40.0f;
}

Eigen::Vector3f FloorScene::light_position() const {
  return {kLightX, light_height_, kLightZ};
}

float FloorScene::irradiance_for(const Eigen::Vector3f& light_pos) const {
  Eigen::Vector3f d = light_pos - circle_center_;
  float r2 = d.squaredNorm();
  float cos_theta = normal_.dot(d.normalized());
  return intensity_ * std::max(0.0f, cos_theta) / r2;
}

float FloorScene::irradiance_at_circle() const {
  return irradiance_for(light_position());
}

void FloorScene::render(rst::rasterizer& r) {
  Eigen::Matrix4f view = camera_.view();
  set_world_lights({light{light_position(),
                          {intensity_, intensity_, intensity_}}});
  set_light_view_matrix(view);

  Eigen::Matrix4f projection = get_projection_matrix(45.0, 1, 0.1, 50);
  view_ = view;
  projection_ = projection;

  r.set_texture(texture_);
  r.set_fragment_shader(diffuse_fragment_shader);
  r.set_model(model_);
  r.set_view(view);
  r.set_projection(projection);
  r.draw(triangles_);
}

void FloorScene::annotate(cv::Mat& image) const {
  auto to_screen = [&](const Eigen::Vector3f& p, cv::Point& out) {
    Eigen::Vector4f pw(p.x(), p.y(), p.z(), 1);
    // Skip points behind the camera (it looks down -z in view space). The
    // sign of clip w depends on the projection convention, so test view z.
    if ((view_ * pw).z() >= 0) return false;
    Eigen::Vector4f c = projection_ * view_ * pw;
    float x = 0.5f * width_ * (c.x() / c.w() + 1.0f);
    float y = 0.5f * height_ * (c.y() / c.w() + 1.0f);
    out = cv::Point(int(x), int(height_ - 1 - y));  // image rows go down
    return true;
  };

  // The light, and a line straight down to where it is above the floor.
  Eigen::Vector3f L = light_position();
  float floor_y = circle_center_.y() -
                  (normal_.x() * (L.x() - circle_center_.x()) +
                   normal_.z() * (L.z() - circle_center_.z())) / normal_.y();
  cv::Point pl, pf;
  if (to_screen(L, pl) && to_screen({L.x(), floor_y, L.z()}, pf)) {
    cv::line(image, pl, pf, cv::Scalar(0, 200, 255), 1, cv::LINE_AA);
    cv::circle(image, pf, 3, cv::Scalar(0, 200, 255), -1, cv::LINE_AA);
    cv::circle(image, pl, 7, cv::Scalar(0, 255, 255), -1, cv::LINE_AA);
    cv::circle(image, pl, 7, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
  }

  char buf[128];
  std::snprintf(buf, sizeof(buf), "light (x, y, z) = (%.2f, %.2f, %.2f)",
                L.x(), L.y(), L.z());
  cv::putText(image, buf, {10, 25}, cv::FONT_HERSHEY_SIMPLEX, 0.6,
              cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
  std::snprintf(buf, sizeof(buf), "irradiance at circle E = %.5f",
                irradiance_at_circle());
  cv::putText(image, buf, {10, 50}, cv::FONT_HERSHEY_SIMPLEX, 0.6,
              cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
}

bool FloorScene::on_key(int key) {
  // Only the camera moves; the light is placed in code (kLightX / kLightZ).
  return camera_.on_key(key);
}

void FloorScene::print_controls() const {
  std::cout << "\n-- Floor scene --\n"
            << "  a / d : orbit camera left / right\n"
            << "  w / s : orbit camera up / down\n"
            << "  TAB   : switch scene\n"
            << "  Esc/q : quit\n";
}
