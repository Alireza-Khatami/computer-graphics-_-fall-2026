#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "rasterizer.hpp"

constexpr double MY_PI = 3.1415926;
inline double DEG2RAD(double deg) { return deg * MY_PI / 180; }

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
  Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1, -eye_pos[2],
      0, 0, 0, 1;

  view = translate * view;

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

Eigen::Matrix4f get_view_matrix_lookat(Eigen::Vector3f eye, Eigen::Vector3f target, Eigen::Vector3f up) {
    Eigen::Vector3f z = (eye - target).normalized();       // camera forward
    Eigen::Vector3f x = up.cross(z).normalized();          // right vector
    Eigen::Vector3f y = z.cross(x);                        // true up vector

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f rotate;
    rotate << x[0], x[1], x[2], 0,
              y[0], y[1], y[2], 0,
              z[0], z[1], z[2], 0,
              0,    0,    0,    1;

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye[0],
                 0, 1, 0, -eye[1],
                 0, 0, 1, -eye[2],
                 0, 0, 0, 1;

    view = rotate * translate;
    return view;
}


int main(int argc, const char** argv) {
  float angle = 0;
  bool command_line = false;
  std::string filename = "output.png";

  printf("argc: %d\n", argc);

  if (argc >= 3) {
    command_line = true;
    angle = std::stof(argv[2]);  // -r by default
    printf("rotating angle: %f\n", angle);
    if (argc == 4) {
      filename = std::string(argv[3]);
    }
  }

  rst::rasterizer r(700, 700);

  Eigen::Vector3f eye_pos = {0, 0, 5};

  std::vector<Eigen::Vector3f> pos{{2, -3, -2}, {0, 4, -4}, {-2, -3, -2}};

  std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

  auto pos_id = r.load_positions(pos);
  auto ind_id = r.load_indices(ind);

  int key = 0;
  int frame_count = 0;

  if (command_line) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.set_model(get_model_matrix(angle));
    r.set_view(get_view_matrix(eye_pos));
    r.set_projection(get_projection_matrix(90, 1, -0.1, -50));

    r.draw(pos_id, ind_id, rst::Primitive::Triangle);
    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);

    printf("saving file %s\n", filename.c_str());
    cv::imwrite(filename, image);

    return 0;
  }
  float eye_fov = 90;
  float obj_eye_angle = 0;
  while (key != 27) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    r.set_model(get_model_matrix(angle));
    eye_pos = {5 * sin(obj_eye_angle), 0, 5 * cos(obj_eye_angle)};
    r.set_view(get_view_matrix_lookat(eye_pos, {0, 0, 0}, {0, 1, 0}));
    r.set_projection(get_projection_matrix(eye_fov, 1, -0.1, -50));

    r.draw(pos_id, ind_id, rst::Primitive::Triangle);

    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::imshow("image", image);
    key = cv::waitKey(10);

    // std::cout << "frame count: " << frame_count++ << '\n';

    if (key == 'a') {
        angle += 3;
    } else if (key == 'd') {
        angle -= 3;
    } else if (key == 'e') {
        // zoom in -> decrease FOV (narrower)
        eye_fov = std::max(10.0f, eye_fov - 2.0f);
    } else if (key == 'q') {
        // zoom out -> increase FOV (wider)
        eye_fov = std::min(150.0f, eye_fov + 2.0f);
    } else if (key == 'w') {
        // move camera up
        obj_eye_angle += 0.2f;
    } else if (key == 's') {
        // move camera down
        obj_eye_angle -= 0.2f;
    }
  }

  return 0;
}
