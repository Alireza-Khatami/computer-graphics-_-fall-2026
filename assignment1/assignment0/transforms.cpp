#include "transforms.hpp"

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

  // Create the model matrix for rotating the triangle around the Z axis.
  float radian_angle = rotation_angle * 3.14f / 180.0;
  model << cos(radian_angle), -sin(radian_angle), 0, 0,
      sin(radian_angle), cos(radian_angle), 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1;

  return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar) {
  Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

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

Eigen::Matrix4f get_model_matrix_transformation(float scale, float rotation_angle, Eigen::Vector3f translation) {
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

  // Combined. Try the other orders (T * R * S, S * R * T, ...) to see how the
  // multiplication order changes the result.
  return T * S * R;
}
