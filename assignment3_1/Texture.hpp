
#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>

#include "global.hpp"
class Texture {
 private:
  cv::Mat image_data;

 public:
  Texture(const std::string& name) {
    image_data = cv::imread(name);
    cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
    width = image_data.cols;
    height = image_data.rows;
  }
  bool is_using_bilinear = true ;
  int width, height;

  Eigen::Vector3f getColor(float u, float v) {
    is_using_bilinear = false ;
    if (u < 0) u = 0;
    if (v < 0) v = 0;
    if (u > 1) u = 1;
    if (v > 1) v = 1;

    auto u_img = u * width;
    auto v_img = (1 - v) * height;

    auto color = image_data.at<cv::Vec3b>(v_img, u_img);
    return Eigen::Vector3f(color[0], color[1], color[2]);
  }
  


   
  Eigen::Vector3f getColorBilinear(float u, float v) {

    // TODO: Implement bilinear texture sampling.
    // Instead of snapping to the nearest pixel (like getColor() does),
    // find the 4 pixels surrounding (u, v) and bilinearly interpolate
    // between them based on how close (u, v) is to each one.
    //
    // Steps:
    //   1. Map (u, v) to image coordinates (u_img, v_img), same as getColor().
    //   2. Find the 4 neighboring pixels:
    //        top-left     = floor(u_img),   floor(v_img)
    //        top-right    = floor(u_img)+1, floor(v_img)
    //        bottom-left  = floor(u_img),   floor(v_img)+1
    //        bottom-right = floor(u_img)+1, floor(v_img)+1
    //      Clamp each coordinate to stay inside the image bounds.
    //   3. Compute the fractional offsets:
    //        s = u_img - floor(u_img)   (horizontal blend factor)
    //        t = v_img - floor(v_img)   (vertical blend factor)
    //   4. Blend the 4 samples:
    //        top    = lerp(top-left,    top-right,    s)
    //        bottom = lerp(bottom-left, bottom-right, s)
    //        result = lerp(top, bottom, t)
    //      where lerp(a, b, f) = a * (1 - f) + b * f
    return Eigen::Vector3f(0, 0, 0);
    }
};
#endif  // RASTERIZER_TEXTURE_H
