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


Eigen::Matrix4f get_model_matrix_SRT(float scale, float rotation_angle, Eigen::Vector3f translation) {
  // Scale matrix
  Eigen::Matrix4f S = Eigen::Matrix4f::Identity();
  S << scale, 0, 0, 0,
       0, scale, 0, 0,
       0, 0, scale, 0,
       0, 0, 0, 1;

  // Rotation matrix (around Z axis)
  float radian_angle = rotation_angle * 3.14f / 180.0f;
  Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
  R << cos(radian_angle), -sin(radian_angle), 0, 0,
       sin(radian_angle), cos(radian_angle), 0, 0,
       0, 0, 1, 0,
       0, 0, 0, 1;

  // Translation matrix
  Eigen::Matrix4f T = Eigen::Matrix4f::Identity();
  T << 1, 0, 0, translation[0],
       0, 1, 0, translation[1],
       0, 0, 1, translation[2],
       0, 0, 0, 1;

  // Combined: T * R * S (translate last, scale first)
  return T * R * S;
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

  Eigen::Vector3f eye_pos = {0, 0, 15};

  Eigen::Vector3f tri_1_color(0.0f, 255.0f, 0.0f);
  Eigen::Vector3f tri_2_color(255.0f, 0.0f, 0.0f);
  // First triangle at origin
  std::vector<Eigen::Vector3f> pos1{{1, -1.5, -2}, {0, 1.5, -2}, {-1, -1.5, -2}};
  std::vector<Eigen::Vector3i> ind1{{0, 1, 2}};
  auto pos_id1 = r.load_positions(pos1);
  auto ind_id1 = r.load_indices(ind1);

  // Second triangle (same shape, will be transformed to (3, 0, 0))
  std::vector<Eigen::Vector3f> pos2{{1, -1.5, -2}, {0, 1.5, -2}, {-1, -1.5, -2}};
  std::vector<Eigen::Vector3i> ind2{{0, 1, 2}};
  auto pos_id2 = r.load_positions(pos2);
  auto ind_id2 = r.load_indices(ind2);

  int key = 0;
  int frame_count = 0;

  // Second triangle transformation parameters
  float tri2_scale = 1.0f;
  float tri2_rotation = 0.0f;
  float tri2_translate_x = 3.0f;

  if (command_line) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.set_model(get_model_matrix(angle));
    r.set_view(get_view_matrix(eye_pos));
    r.set_projection(get_projection_matrix(45, 1, -0.1, -50));
    r.draw(pos_id1, ind_id1, rst::Primitive::Triangle, tri_1_color);


    r.set_model(get_model_matrix_SRT(tri2_scale, tri2_rotation, {tri2_translate_x, 0, 0}));
    r.draw(pos_id2, ind_id2, rst::Primitive::Triangle , tri_2_color);

    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);

    printf("saving file %s\n", filename.c_str());
    cv::imwrite(filename, image);

    return 0;
  }
  float eye_fov = 45;
  float obj_eye_angle = 0;
  while (key != 27) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);


    // Draw first triangle (at origin, controlled by a/d)
    r.set_model(get_model_matrix(angle));
    eye_pos = {15 * sin(obj_eye_angle), 0, 15 * cos(obj_eye_angle)};
    r.set_view(get_view_matrix_lookat(eye_pos, {1.5, 0, 0}, {0, 1, 0}));
    r.set_projection(get_projection_matrix(eye_fov, 1, -0.1, -50));
    r.draw(pos_id1, ind_id1, rst::Primitive::Triangle, tri_1_color);

    // Draw second triangle (at (3, 0, 0), controlled by j/k, o/i, n/m)
    r.set_model(get_model_matrix_SRT(tri2_scale, tri2_rotation, {tri2_translate_x, 0, 0}));
    r.draw(pos_id2, ind_id2, rst::Primitive::Triangle, tri_2_color);

    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::imshow("image", image);
    key = cv::waitKey(10);

    // std::cout << "frame count: " << frame_count++ << '\n';

    // First triangle controls
     if (key == 'w') {
        // zoom in -> decrease FOV (narrower)
        eye_fov = std::max(10.0f, eye_fov - 2.0f);
    } else if (key == 's') {
        // zoom out -> increase FOV (wider)
        eye_fov = std::min(150.0f, eye_fov + 2.0f);
    } else if (key == 'd') {
        // move camera around
        obj_eye_angle += 0.2f;
    } else if (key == 'a') {
        // move camera around
        obj_eye_angle -= 0.2f;
    }
    // Second triangle controls
    else if (key == 'k') {
        // scale up
        tri2_scale += 0.1f;
    } else if (key == 'j') {
        // scale down
        tri2_scale = std::max(0.1f, tri2_scale - 0.1f);
    } else if (key == 'o') {
        // rotate clockwise
        tri2_rotation += 5.0f;
    } else if (key == 'i') {
        // rotate counter-clockwise
        tri2_rotation -= 5.0f;
    } else if (key == 'm') {
        // translate right
        tri2_translate_x += 0.5f;
    } else if (key == 'n') {
        // translate left
        tri2_translate_x -= 0.5f;
    }
  }

  return 0;
}
