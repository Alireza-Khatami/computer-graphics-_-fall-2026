
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
  // Eigen::Vector3f getColorBilinear(float u, float v) {
  //   // Your code here
  //   return Eigen::Vector3f(0, 0, 0);
  // }
  Eigen::Vector3f getColorBilinear(float u, float v) {
    is_using_bilinear = true;
    if (u < 0) u = 0;
    if (v < 0) v = 0;
    if (u > 1) u = 1;
    if (v > 1) v = 1;

    const float u_img = u * width;
    const float v_img = (1 - v) * height;

    int x0 = static_cast<int>(u_img);
    int y0 = static_cast<int>(v_img);
    if (x0 >= width) x0 = width - 1;
    if (y0 >= height) y0 = height - 1;

    int x1 = x0 + 1;
    int y1 = y0 + 1;
    if (x1 >= width) x1 = width - 1;
    if (y1 >= height) y1 = height - 1;

    const float s = u_img - static_cast<float>(x0);
    const float t = v_img - static_cast<float>(y0);
    const float one_minus_s = 1.0f - s;
    const float one_minus_t = 1.0f - t;

    const cv::Vec3b c00 = image_data.at<cv::Vec3b>(y0, x0);
    const cv::Vec3b c10 = image_data.at<cv::Vec3b>(y0, x1);
    const cv::Vec3b c01 = image_data.at<cv::Vec3b>(y1, x0);
    const cv::Vec3b c11 = image_data.at<cv::Vec3b>(y1, x1);

    return Eigen::Vector3f(
        one_minus_t * (one_minus_s * c00[0] + s * c10[0]) +
            t * (one_minus_s * c01[0] + s * c11[0]),
        one_minus_t * (one_minus_s * c00[1] + s * c10[1]) +
            t * (one_minus_s * c01[1] + s * c11[1]),
        one_minus_t * (one_minus_s * c00[2] + s * c10[2]) +
            t * (one_minus_s * c01[2] + s * c11[2]));
  }
};
#endif  // RASTERIZER_TEXTURE_H
