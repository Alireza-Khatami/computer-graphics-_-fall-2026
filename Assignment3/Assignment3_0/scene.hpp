#pragma once

#include <opencv2/opencv.hpp>

#include "rasterizer.hpp"

// Common interface for every scene. main.cpp keeps one instance of each
// scene alive and switches between them with TAB, so a scene owns its own
// state (camera, model, lights, ...).
class Scene {
 public:
  virtual ~Scene() = default;

  // Short name shown in the window title / console.
  virtual const char* name() const = 0;

  // Draw one frame into the rasterizer (the caller already cleared the
  // buffers). A scene sets its own shader, texture, lights and transforms.
  virtual void render(rst::rasterizer& r) = 0;

  // Optional 2D overlay drawn on top of the finished frame (BGR image).
  virtual void annotate(cv::Mat& image) const {}

  // React to a key press. Return true if the key was consumed.
  virtual bool on_key(int key) = 0;

  // Print the scene specific controls to the console.
  virtual void print_controls() const = 0;
};
