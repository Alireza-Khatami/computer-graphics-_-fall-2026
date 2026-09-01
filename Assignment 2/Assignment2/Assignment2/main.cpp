#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <string>

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
    float radian_angle = rotation_angle * 3.14f / 180.0;
    model << cos(radian_angle), -sin(radian_angle), 0, 0,
        sin(radian_angle), cos(radian_angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    return model;
}
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
    float zNear, float zFar) {
    // Students will implement this function
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    float rad_fov = eye_fov * 3.14f / 180.0;
    float t = tan(rad_fov / 2) * fabs(zNear);
    float r = t * aspect_ratio;
    // Perspective to orthographic 
    Eigen::Matrix4f persp_to_ortho;
    persp_to_ortho << zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, -zNear * zFar,
        0, 0, 1, 0;
    Eigen::Matrix4f ortho;
    ortho << 1 / r, 0, 0, 0,
        0, 1 / t, 0, 0,
        0, 0, 2 / (zNear - zFar), (zNear + zFar) / (zNear - zFar),
        0, 0, 0, 1;
    projection = ortho * persp_to_ortho;
    return projection;
}

void generate_validate_images(rst::rasterizer& r,
    rst::pos_buf_id pos_id, rst::ind_buf_id ind_id, rst::col_buf_id col_id,
    Eigen::Vector3f eye_pos, float angle, const std::string& base_filename) {
    // Strip extension from base_filename to use as prefix
    std::string prefix = base_filename;
    auto dot = prefix.rfind('.');
    if (dot != std::string::npos) prefix = prefix.substr(0, dot);
    struct Config {
        rst::CullMode   cull;
        int             blend; // 0=depth_test, 1=src_alpha, 2=one
        std::string     label;
    };
    std::vector<Config> configs = {
        { rst::CullMode::None,  0, "0_none_depth_test"  },
        { rst::CullMode::None,  1, "1_none_src_alpha"   },
        { rst::CullMode::None,  2, "2_none_one"         },
        { rst::CullMode::Back,  0, "3_back_depth_test"  },
        { rst::CullMode::Front, 0, "4_front_depth_test" },
    };
    for (auto& cfg : configs) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        r.set_cull_mode(cfg.cull);
        r.set_alpha_blend_mode(cfg.blend);
        r.set_model(get_model_matrix(0));
        r.set_view(get_view_matrix(eye_pos, angle));
        r.set_projection(get_projection_matrix(90, 1, -0.1, -50));
        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        std::vector<Vector3f> frame_buffer_color;
        for (auto& v : r.frame_buffer())
            frame_buffer_color.push_back(Vector3f(v[0], v[1], v[2]));
        cv::Mat image(700, 700, CV_32FC3, frame_buffer_color.data());
        image.convertTo(image, CV_8UC3, 1.0f);
        std::string out = prefix + "_" + cfg.label + ".png";
        printf("saving %s\n", out.c_str());
        cv::imwrite(out, image);
    }
}

int main(int argc, const char** argv) {
  float angle = 0;
  std::string filename = "output.png";
  rst::rasterizer r(700, 700);
  Eigen::Vector3f eye_pos = {0, 0, 10};
  std::vector<Eigen::Vector3f> pos{{2, -3, -3},    {0, 4, -3},
                                   {-2, -3, -3},   {3.5, -1, -3.5},
                                   {2.5, 1.5, -3.5}, {-1, 0.5, -3.5},
                                   //Front-facing triangle (CCW winding)
                                   {-4, 2, -3}, {-3, 4, -3}, {-5, 4, -3},
                                   // Back-facing triangle (CW winding)
                                   {4, -1 , -3}, {5, 2, -3}, {3, 2, -3}};

  std::vector<Eigen::Vector3i> ind{{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {11, 10, 9}};
  std::vector<float> alpha = {0.5f, 0.5f};
  std::vector<Eigen::Vector4f> cols{{232, 117, 166, alpha[0]}, {232, 117, 166, alpha[0]},
                                     {232, 117, 166, alpha[0]}, {0, 100, 0, alpha[1]},
                                     {0, 100, 0, alpha[1]},     {0, 100, 0, alpha[1]},
                                     // Front-facing triangle (blue, alpha=1)
                                     {0, 100, 255, 1.0f}, {0, 100, 255, 1.0f}, {0, 100, 255, 1.0f},
                                     // Back-facing triangle (yellow, alpha=1)
                                     {255, 255, 0, 1.0f}, {255, 255, 0, 1.0f}, {255, 255, 0, 1.0f}};
  auto pos_id = r.load_positions(pos);
  auto ind_id = r.load_indices(ind);
  auto col_id = r.load_colors(cols);
  int key = 0;
  // Command-line mode: ./Rasterizer <blend_mode> <cull_mode> <output_file>
  if (argc >= 4) {
    int blend_mode_arg = std::stoi(std::string(argv[1]));
    int cull_mode_arg  = std::stoi(std::string(argv[2]));
    filename = std::string(argv[3]);

    r.set_alpha_blend_mode(blend_mode_arg);
    r.set_cull_mode(cull_mode_arg);

    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    r.set_model(get_model_matrix(0));
    r.set_view(get_view_matrix(eye_pos, angle));
    r.set_projection(get_projection_matrix(90, 1, -0.1, -50));
    r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

    std::vector<Vector3f> frame_buffer_color;
    for (auto& v : r.frame_buffer())
      frame_buffer_color.push_back(Vector3f(v[0], v[1], v[2]));

    cv::Mat image(700, 700, CV_32FC3, frame_buffer_color.data());
    image.convertTo(image, CV_8UC3, 1.0f);
    printf("saving file %s\n", filename.c_str());
    cv::imwrite(filename, image);
    return 0;
  }

  // Validate mode: ./Rasterizer validate <output_prefix>
  if (argc >= 2 && std::string(argv[1]) == "validate") {
    filename = (argc >= 3) ? std::string(argv[2]) : "output.png";
    generate_validate_images(r, pos_id, ind_id, col_id, eye_pos, angle, filename);
    return 0;
  }

  // Interactive (GUI) mode
  while (key != 27) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    r.set_model(get_model_matrix(0));
    r.set_view(get_view_matrix(eye_pos, angle));
    r.set_projection(get_projection_matrix(90, 1, -0.1, -50));
    r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

    std::vector<Vector3f> frame_buffer_color;
    for (auto& v : r.frame_buffer())
      frame_buffer_color.push_back(Vector3f(v[0], v[1], v[2]));

    cv::Mat image(700, 700, CV_32FC3, frame_buffer_color.data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::imshow("image", image);
    key = cv::waitKey(10);

    if (key == 'a')
      angle += 10;
    else if (key == 'd')
      angle -= 10;
  }

  return 0;
}





//     std::vector<Config> configs = {
//  none-depth-test  
//  none-src-alpha   
//  none-one         
//  back-depth-test  
//  front-depth-test 
//     };