#pragma once

#include "rasterizer.hpp"

// Common interface for every scene in this assignment. main.cpp keeps one
// instance of each scene alive and switches between them with the keyboard,
// so a scene owns its own state (camera, angles, blend mode, ...).
class Scene {
 public:
  virtual ~Scene() = default;

  // Short name shown in the console.
  virtual const char* name() const = 0;

  // Draw one frame into the rasterizer (the caller already cleared the buffers).
  virtual void render(rst::rasterizer& r) = 0;

  // React to a key press. Return true if the key was consumed by the scene.
  virtual bool on_key(int key) = 0;

  // Print the scene specific controls to the console.
  virtual void print_controls() const = 0;
};
