#include <Eigen/Eigen>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "cube_scene.hpp"
#include "rasterizer.hpp"
#include "scene.hpp"
#include "transforms.hpp"
#include "triangle_scene.hpp"

namespace {

constexpr int kWidth = 700;
constexpr int kHeight = 700;
constexpr char kWindowName[] = "CS 6366 - Assignment 1";

void print_usage(const char* exe) {
  printf(
      "usage:\n"
      "  %s                            interactive, starts on the triangle scene\n"
      "  %s --scene cube               interactive, starts on the cube scene\n"
      "  %s -r <angle> [out.png]       headless render of the triangle scene\n"
      "  %s --scene <name> <out.png>   headless render of the given scene\n",
      exe, exe, exe, exe);
}

void save_frame(rst::rasterizer& r, const std::string& filename) {
  cv::Mat image(r.get_height(), r.get_width(), CV_32FC3,
                r.frame_buffer().data());
  image.convertTo(image, CV_8UC3, 1.0f);
  printf("saving file %s\n", filename.c_str());
  cv::imwrite(filename, image);
}

}  // namespace

int main(int argc, const char** argv) {
  rst::rasterizer r(kWidth, kHeight);

  // Every scene loads its geometry into the rasterizer once, up front.
  TriangleScene triangle_scene(r);
  CubeScene cube_scene(r);

  std::vector<Scene*> scenes{&triangle_scene, &cube_scene};
  int current = 0;

  // ---------------------------------------------------------------------
  // Command line parsing
  // ---------------------------------------------------------------------
  bool command_line = false;
  std::string filename = "output.png";

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];

    if (arg == "--scene" && i + 1 < argc) {
      const std::string scene_name = argv[++i];
      if (scene_name == "cube") {
        current = 1;
      } else if (scene_name == "triangle") {
        current = 0;
      } else {
        printf("unknown scene '%s' (expected 'triangle' or 'cube')\n",
               scene_name.c_str());
        return 1;
      }
      // An extra argument after the scene name means "render and exit".
      if (i + 1 < argc) {
        command_line = true;
        filename = argv[++i];
      }
    } else if (arg == "-r" && i + 1 < argc) {
      // Backwards compatible headless path: rotate the first triangle.
      command_line = true;
      const float angle = std::stof(argv[++i]);
      printf("rotating angle: %f\n", angle);
      triangle_scene.set_first_triangle_angle(angle);
      current = 0;
      if (i + 1 < argc) {
        filename = argv[++i];
      }
    } else if (arg == "-h" || arg == "--help") {
      print_usage(argv[0]);
      return 0;
    } else {
      printf("unrecognised argument '%s'\n", arg.c_str());
      print_usage(argv[0]);
      return 1;
    }
  }

  // ---------------------------------------------------------------------
  // Headless render
  // ---------------------------------------------------------------------
  if (command_line) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    scenes[current]->render(r);
    save_frame(r, filename);
    return 0;
  }

  // ---------------------------------------------------------------------
  // Interactive loop
  // ---------------------------------------------------------------------
  printf("\n=== CS 6366 - Assignment 1 ===\n");
  printf("  1 / 2 or TAB : switch scene      ESC : quit\n");
  printf("current scene: %s\n", scenes[current]->name());
  scenes[current]->print_controls();

  int key = 0;
  while (key != 27) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    scenes[current]->render(r);

    cv::Mat image(kHeight, kWidth, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::imshow(kWindowName, image);
    key = cv::waitKey(10);

    // Global keys first, then hand the key to the active scene.
    const int previous = current;
    if (key == '1') {
      current = 0;
    } else if (key == '2') {
      current = 1;
    } else if (key == 9) {  // TAB
      current = (current + 1) % static_cast<int>(scenes.size());
    } else if (key > 0) {
      scenes[current]->on_key(key);
    }

    if (current != previous) {
      printf("\ncurrent scene: %s\n", scenes[current]->name());
      scenes[current]->print_controls();
    }
  }

  return 0;
}
