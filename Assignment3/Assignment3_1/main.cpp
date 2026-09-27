
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "floor_scene.hpp"
#include "global.hpp"
#include "rasterizer.hpp"
#include "scene.hpp"
#include "shaders.hpp"
#include "spot_scene.hpp"
#include "transforms.hpp"

// LookAt view matrix: places the camera at `eye_pos`, always facing the
// origin (where the model sits), with +Y as up.
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
  Eigen::Vector3f target(0, 0, 0);
  Eigen::Vector3f up(0, 1, 0);
  Eigen::Vector3f z_axis = (eye_pos - target).normalized();  // target -> eye
  Eigen::Vector3f x_axis = up.cross(z_axis).normalized();
  Eigen::Vector3f y_axis = z_axis.cross(x_axis);

  Eigen::Matrix4f view;
  view << x_axis.x(), x_axis.y(), x_axis.z(), -x_axis.dot(eye_pos),
          y_axis.x(), y_axis.y(), y_axis.z(), -y_axis.dot(eye_pos),
          z_axis.x(), z_axis.y(), z_axis.z(), -z_axis.dot(eye_pos),
          0,          0,          0,           1;
  return view;
}

// Camera position on a sphere of the given radius around the origin.
// yaw orbits around the Y axis (a/d), pitch tilts up/down (w/s).
// yaw = pitch = 0 gives (0, 0, radius), the original fixed camera.
Eigen::Vector3f orbit_eye_pos(float radius, float yaw_deg, float pitch_deg) {
  float yaw = yaw_deg * MY_PI / 180.f;
  float pitch = pitch_deg * MY_PI / 180.f;
  return Eigen::Vector3f(radius * cos(pitch) * sin(yaw),
                         radius * sin(pitch),
                         radius * cos(pitch) * cos(yaw));
}

Eigen::Matrix4f get_model_matrix(float angle) {
  Eigen::Matrix4f rotation;
  angle = angle * MY_PI / 180.f;
  rotation << cos(angle), 0, sin(angle), 0, 0, 1, 0, 0, -sin(angle), 0,
      cos(angle), 0, 0, 0, 0, 1;

  Eigen::Matrix4f scale;
  scale << 2.5, 0, 0, 0, 0, 2.5, 0, 0, 0, 0, 2.5, 0, 0, 0, 0, 1;

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;

  return translate * rotation * scale;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar) {
  // TODO: Use the same projection matrix from the previous assignments.
  // TODO: Implement this function
  // Create the projection matrix for the given parameters.
  // Then return it.
  return Eigen::Matrix4f::Identity();
}


namespace {

// Paths are relative to the build folder the executable runs from.
const std::string kSpotObj = "../models/spot/spot_triangulated_good.obj";
const std::string kSpotTexture = "../models/spot/spot_texture.png";
const std::string kFloorTexture = "../models/floor/floor_texture.png";

// Frame buffer (float RGB, 0..255) -> 8-bit BGR image for OpenCV.
cv::Mat frame_to_image(rst::rasterizer& r) {
  cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
  image.convertTo(image, CV_8UC3, 1.0f);
  cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
  return image;
}

void render_scene(rst::rasterizer& r, Scene& scene, cv::Mat& image) {
  r.clear(rst::Buffers::Color | rst::Buffers::Depth);
  scene.render(r);
  image = frame_to_image(r);
  scene.annotate(image);
}

void print_sampling_mode(rst::rasterizer& r) {
  if (!r.texture) return;
  if (r.texture->is_using_bilinear)
    std::cout << "Bilinear interpolation is used for texture sampling.\n";
  else
    std::cout << "Nearest neighbor sampling is used for texture sampling.\n";
}

}  // namespace

int main(int argc, const char** argv) {
  rst::rasterizer r(700, 700);
  r.set_vertex_shader(vertex_shader);
  cv::Mat image;

  // Floor scene, headless (light position comes from floor_scene.cpp):
  //   ./Rasterizer floor <output.png>
  if (argc >= 2 && std::string(argv[1]) == "floor") {
    FloorScene floor(kFloorTexture);
    if (argc < 3) {
      std::cout << "usage: ./Rasterizer floor <output.png>\n";
      return 1;
    }
    render_scene(r, floor, image);
    Eigen::Vector3f L = floor.light_position();
    std::cout << "light = (" << L.x() << ", " << L.y() << ", " << L.z()
              << "), irradiance at circle E = " << floor.irradiance_at_circle()
              << "\n";
    cv::imwrite(argv[2], image);
    return 0;
  }

  // Model, headless (fixed camera):
  //   ./Rasterizer <output.png> <model.obj>                -> normal shader
  //   ./Rasterizer <output.png> <model.obj> <texture.png>  -> texture shader
  if (argc == 3 || argc == 4) {
    std::string filename = argv[1];
    std::string obj_path = argv[2];
    std::string texture_path = (argc == 4) ? argv[3] : "";
    SpotScene spot(obj_path, texture_path);
    if (argc == 3) {
      std::cout << "Rasterizing using the normal shader\n";
      spot.set_fragment_shader(normal_fragment_shader);
    } else {
      std::cout << "Rasterizing using the texture shader\n";
    }
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    spot.render(r);
    if (argc == 4) print_sampling_mode(r);
    cv::imwrite(filename, frame_to_image(r));
    return 0;
  }

  // Interactive mode (no arguments): TAB switches between the scenes.
  SpotScene spot(kSpotObj, kSpotTexture);
  FloorScene floor(kFloorTexture);
  std::vector<Scene*> scenes{&spot, &floor};
  int current = 0;
  scenes[current]->print_controls();

  int key = 0;
  while (key != 27 && key != 'q') {
    render_scene(r, *scenes[current], image);
    cv::imshow("image", image);
    cv::imwrite("output.png", image);
    key = cv::waitKey(0);

    if (key == '\t') {
      current = (current + 1) % scenes.size();
      scenes[current]->print_controls();
    } else if (key > 0) {
      scenes[current]->on_key(key);
    }
  }
  return 0;
}
