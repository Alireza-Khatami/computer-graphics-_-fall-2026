#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "Triangle.hpp"
#include "cube_scene.hpp"
#include "rasterizer.hpp"
#include "scene.hpp"
#include "transforms.hpp"
#include "triangle_scene.hpp"

constexpr double MY_PI = 3.1415926;
inline double DEG2RAD(double deg) { return deg * MY_PI / 180; }

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos, float angle) {
  float rad = angle * 3.14f / 180.0f;
  float r = eye_pos.norm();
  // Camera orbits on a sphere of radius r around the Y axis,
  // always looking at the origin. The view matrix is the inverse
  // of the camera's world transform (lookAt with target = origin).
  Eigen::Matrix4f view;
  view << cos(rad),  0, -sin(rad),  0,
          0,         1,  0,         0,
          sin(rad),  0,  cos(rad), -r,
          0,         0,  0,         1;
  return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle) {
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    return model;
}
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
    float zNear, float zFar) {
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    return projection;
}

namespace {

void save_frame(rst::rasterizer& r, const std::string& filename) {
    std::vector<Vector3f> frame_buffer_color;
    for (auto& v : r.frame_buffer())
        frame_buffer_color.push_back(Vector3f(v[0], v[1], v[2]));
    cv::Mat image(700, 700, CV_32FC3, frame_buffer_color.data());
    image.convertTo(image, CV_8UC3, 1.0f);
    printf("saving %s\n", filename.c_str());
    cv::imwrite(filename, image);
}

void generate_validate_images(rst::rasterizer& r, TriangleScene& triangle,
    const std::string& base_filename) {
    // Strip extension from base_filename to use as prefix
    std::string prefix = base_filename;
    auto dot = prefix.rfind('.');
    if (dot != std::string::npos) prefix = prefix.substr(0, dot);
    struct Config {
        int cull;   // 0=None, 1=Back, 2=Front
        int blend;  // 0=depth_test, 1=src_alpha, 2=one
        std::string label;
    };
    std::vector<Config> configs = {
        { 0, 0, "0_none_depth_test"  },
        { 0, 1, "1_none_src_alpha"   },
        { 0, 2, "2_none_one"         },
        { 1, 0, "3_back_depth_test"  },
        { 2, 0, "4_front_depth_test" },
    };
    for (auto& cfg : configs) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        triangle.render_with(r, cfg.blend, cfg.cull);
        save_frame(r, prefix + "_" + cfg.label + ".png");
    }
}

}  // namespace

int main(int argc, const char** argv) {
  rst::rasterizer r(700, 700);

  // Every scene loads its geometry into the rasterizer once, up front.
  TriangleScene triangle_scene(r);
  CubeScene cube_scene(r);
  std::vector<Scene*> scenes{&triangle_scene, &cube_scene};
  int current = 0;

  std::string filename = "output.png";

  // Cube mode: ./Rasterizer cube <output_file>
  if (argc >= 2 && std::string(argv[1]) == "cube") {
    filename = (argc >= 3) ? std::string(argv[2]) : "cube.png";
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    cube_scene.render(r);
    save_frame(r, filename);
    return 0;
  }

  // Command-line mode: ./Rasterizer <blend_mode> <cull_mode> <output_file>
  if (argc >= 4) {
    int blend_mode_arg = std::stoi(std::string(argv[1]));
    int cull_mode_arg  = std::stoi(std::string(argv[2]));
    filename = std::string(argv[3]);

    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    triangle_scene.render_with(r, blend_mode_arg, cull_mode_arg);
    save_frame(r, filename);
    return 0;
  }

  // Validate mode: ./Rasterizer validate <output_prefix>
  if (argc >= 2 && std::string(argv[1]) == "validate") {
    filename = (argc >= 3) ? std::string(argv[2]) : "output.png";
    generate_validate_images(r, triangle_scene, filename);
    return 0;
  }

  // Interactive (GUI) mode
  scenes[current]->print_controls();

  int key = 0;
  while (key != 27) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    scenes[current]->render(r);

    std::vector<Vector3f> frame_buffer_color;
    for (auto& v : r.frame_buffer())
      frame_buffer_color.push_back(Vector3f(v[0], v[1], v[2]));

    cv::Mat image(700, 700, CV_32FC3, frame_buffer_color.data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::imshow("image", image);
    key = cv::waitKey(10);

    if (key == '\t') {
      current = 1 - current;
      scenes[current]->print_controls();
    } else if (key > 0) {
      scenes[current]->on_key(key);
    }
  }

  return 0;
}
