#pragma once

#include <opencv2/opencv.hpp>

// Common interface for every scene. main.cpp keeps one instance of each
// scene alive and switches between them with TAB, so a scene owns its own
// state (control points, view, ...).
class Scene {
 public:
    virtual ~Scene() = default;

    // Short name shown in the console.
    virtual const char* name() const = 0;

    // Draw one frame into a black kWidth x kHeight BGR image.
    virtual void render(cv::Mat& image) const = 0;

    // React to a key press. Return true if the key was consumed.
    virtual bool on_key(int key) = 0;

    // Print the scene specific controls to the console.
    virtual void print_controls() const = 0;
};
