#include "triangle_scene.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "transforms.hpp"

namespace {
constexpr float kNear = -0.1f;
constexpr float kFar = -50.0f;

// Where primitives are clipped. This is the plane of the camera itself rather
// than the near plane of the projection, so nothing disappears before it
// actually reaches the eye. It cannot be exactly 0 - see rasterizer::set_near.
constexpr float kClipPlane = -1e-4f;
}  // namespace

TriangleScene::TriangleScene(rst::rasterizer& r) {
  // First triangle at the origin.
  std::vector<Eigen::Vector3f> pos1{{1, -1.5, -2}, {0, 1.5, -2}, {-1, -1.5, -2}};
  std::vector<Eigen::Vector3i> ind1{{0, 1, 2}};
  pos_id1_ = r.load_positions(pos1);
  ind_id1_ = r.load_indices(ind1);

  // Second triangle (same shape, will be transformed to (3, 0, 0)).
  std::vector<Eigen::Vector3f> pos2{{1, -1.5, -2}, {0, 1.5, -2}, {-1, -1.5, -2}};
  std::vector<Eigen::Vector3i> ind2{{0, 1, 2}};
  pos_id2_ = r.load_positions(pos2);
  ind_id2_ = r.load_indices(ind2);
}

void TriangleScene::render(rst::rasterizer& r) {
  const float aspect =
      static_cast<float>(r.get_width()) / static_cast<float>(r.get_height());

  // Green triangle: orbiting look-at camera.
  Eigen::Vector3f eye_pos = {15 * std::sin(obj_eye_angle_), 0,
                             15 * std::cos(obj_eye_angle_)};
  r.set_model(get_model_matrix(angle_));
  r.set_view(get_view_matrix_lookat(eye_pos, {1.5, 0, 0}, {0, 1, 0}));
  r.set_near(kClipPlane);
  r.set_projection(get_projection_matrix(eye_fov_, aspect, kNear, kFar));
  r.draw(pos_id1_, ind_id1_, rst::Primitive::Triangle, tri_1_color_);

  // Blue triangle: composed model transformation.
  r.set_model(get_model_matrix_transformation(tri2_scale_, tri2_rotation_,
                                              {tri2_translate_x_, 0, 0}));
  r.draw(pos_id2_, ind_id2_, rst::Primitive::Triangle, tri_2_color_);
}

bool TriangleScene::on_key(int key) {
  switch (key) {
    // Camera controls.
    case 'w':  // zoom in -> decrease FOV (narrower)
      eye_fov_ = std::max(10.0f, eye_fov_ - 2.0f);
      return true;
    case 's':  // zoom out -> increase FOV (wider)
      eye_fov_ = std::min(150.0f, eye_fov_ + 2.0f);
      return true;
    case 'd':  // orbit the camera
      obj_eye_angle_ += 0.2f;
      return true;
    case 'a':
      obj_eye_angle_ -= 0.2f;
      return true;

    // Second triangle controls.
    case 'k':  // scale up
      tri2_scale_ += 0.1f;
      return true;
    case 'j':  // scale down
      tri2_scale_ = std::max(0.1f, tri2_scale_ - 0.1f);
      return true;
    case 'o':  // rotate clockwise
      tri2_rotation_ += 5.0f;
      return true;
    case 'i':  // rotate counter-clockwise
      tri2_rotation_ -= 5.0f;
      return true;
    case 'm':  // translate right
      tri2_translate_x_ += 0.5f;
      return true;
    case 'n':  // translate left
      tri2_translate_x_ -= 0.5f;
      return true;
    default:
      return false;
  }
}

void TriangleScene::print_controls() const {
  printf(
      "\n[Triangle scene]\n"
      "  camera : a / d  orbit      w / s  field of view\n"
      "  blue   : j / k  scale      i / o  rotate       n / m  translate\n");
}
