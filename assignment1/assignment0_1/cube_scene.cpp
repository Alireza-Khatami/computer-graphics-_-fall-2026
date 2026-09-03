#include "cube_scene.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "transforms.hpp"

namespace {
// Near / far planes of the camera. They are negative because the camera looks
// down the -Z axis (see the "n > f" caveat in the lecture slides).
constexpr float kNear = -0.1f;
constexpr float kFar = -50.0f;

// Where primitives are clipped. This is the plane of the camera itself rather
// than the near plane of the projection, so nothing disappears before it
// actually reaches the eye. It cannot be exactly 0 - see rasterizer::set_near.
constexpr float kClipPlane = -1e-4f;
}  // namespace

CubeScene::CubeScene(rst::rasterizer& r) {
  // Axis aligned cube centred on the origin, side length 2.
  std::vector<Eigen::Vector3f> pos{
      {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},  // back face  (z = -1)
      {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1},   // front face (z = +1)
  };

  // Six quads, one per face, each listed counter clockwise seen from outside.
  // Using quads instead of triangles means the wireframe shows exactly the
  // twelve edges of the cube, with no diagonal across the faces.
  std::vector<Eigen::Vector4i> ind{
      {4, 5, 6, 7},  // front  (+z)
      {1, 0, 3, 2},  // back   (-z)
      {0, 4, 7, 3},  // left   (-x)
      {5, 1, 2, 6},  // right  (+x)
      {3, 7, 6, 2},  // top    (+y)
      {0, 1, 5, 4},  // bottom (-y)
  };

  pos_id_ = r.load_positions(pos);
  ind_id_ = r.load_indices(ind);
}

void CubeScene::render(rst::rasterizer& r) {
  const float aspect =
      static_cast<float>(r.get_width()) / static_cast<float>(r.get_height());

  Eigen::Vector3f eye_pos = {eye_dist_ * std::sin(orbit_), eye_height_,
                             eye_dist_ * std::cos(orbit_)};

  r.set_model(get_model_matrix(spin_));
  r.set_view(get_view_matrix_lookat(eye_pos, {0, 0, 0}, {0, 1, 0}));
  r.set_near(kClipPlane);
  r.set_projection(get_projection_matrix(eye_fov_, aspect, kNear, kFar));
  r.draw(pos_id_, ind_id_, rst::Primitive::Quad, cube_color_);
}

bool CubeScene::on_key(int key) {
  switch (key) {
    case 'a':  // orbit the camera around the cube
      orbit_ -= 0.1f;
      return true;
    case 'd':
      orbit_ += 0.1f;
      return true;
    case 'r':  // raise / lower the camera
      eye_height_ += 0.2f;
      return true;
    case 'f':
      eye_height_ -= 0.2f;
      return true;
    case 'w':  // dolly: move the camera towards / away from the cube
      eye_dist_ = std::max(0.25f, eye_dist_ - 0.25f);
      print_camera();
      return true;
    case 's':
      eye_dist_ = std::min(40.0f, eye_dist_ + 0.25f);
      print_camera();
      return true;
    case 'z':  // change the field of view without moving the camera
      eye_fov_ = std::max(10.0f, eye_fov_ - 5.0f);
      print_camera();
      return true;
    case 'x':
      eye_fov_ = std::min(150.0f, eye_fov_ + 5.0f);
      print_camera();
      return true;
    case 'o':  // spin the cube around the Z axis
      spin_ += 5.0f;
      return true;
    case 'i':
      spin_ -= 5.0f;
      return true;
    case 'p':
      print_camera();
      return true;
    default:
      return false;
  }
}

void CubeScene::print_camera() const {
  // Half of the vertical extent of the world that is visible at the distance
  // of the cube. Keeping this value constant while changing the field of view
  // is exactly what a "dolly zoom" does.
  const float half_height =
      eye_dist_ * std::tan(static_cast<float>(DEG2RAD(eye_fov_ / 2.0)));
  printf("[Cube] fovY = %6.1f deg   distance = %5.2f   d*tan(fov/2) = %6.3f\n",
         eye_fov_, eye_dist_, half_height);
}

void CubeScene::print_controls() const {
  printf(
      "\n[Cube scene]\n"
      "  camera : a / d  orbit      r / f  height\n"
      "           w / s  dolly (move closer / further away)\n"
      "           z / x  field of view (zoom, camera does not move)\n"
      "  cube   : i / o  spin around Z\n"
      "  p      : print the current camera parameters\n");
}
