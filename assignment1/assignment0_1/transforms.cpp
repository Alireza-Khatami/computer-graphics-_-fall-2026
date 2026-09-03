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

Eigen::Matrix4f get_projection_matrix(float eye_fov,
                                      float aspect_ratio,
                                      float zNear,
                                      float zFar)
{
    // TODO 1: Convert the field-of-view from degrees to radians.
    // Hint: Trigonometric functions in C++ expect radians.
    // float rad_fov = ...

    // TODO 2: Compute the top (t) and right (r) values of the near plane.
    // Hint: Use tan(fov / 2) and the absolute value of zNear.
    //       Remember that the near plane is NOT square unless aspect_ratio == 1.
    // float t = ...
    // float r = ...

    // TODO 3: Construct the perspective-to-orthographic projection matrix.
    // This matrix converts the frustum into a cuboid.
    Eigen::Matrix4f persp_to_ortho = Eigen::Matrix4f::Identity();
    // Fill in the matrix elements

    // TODO 4: Construct the orthographic projection matrix.
    // This matrix maps the cuboid into normalized device coordinates.
    Eigen::Matrix4f ortho = Eigen::Matrix4f::Identity();
    // Fill in the matrix elements

    // TODO 5: Combine the orthographic and perspective matrices.
    // Hint: The order of multiplication matters.
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    // projection = ...

    return projection;
}

Eigen::Matrix4f get_view_matrix_lookat(Eigen::Vector3f eye,
                                      Eigen::Vector3f target,
                                      Eigen::Vector3f up)
{
    // TODO 1: Compute the camera forward (z) vector.
    // Hint: It should point from the camera position toward the scene.
    // Eigen::Vector3f z = ...

    // TODO 2: Compute the camera right (x) vector.
    // Hint: Use the cross product between the up vector and the forward vector.
    // Eigen::Vector3f x = ...

    // TODO 3: Compute the camera true up (y) vector.
    // Hint: Use the cross product between the forward and right vectors.
    // Eigen::Vector3f y = ...

    // TODO 4: Construct the rotation matrix using the camera basis vectors.
    // The rotation matrix should align world coordinates with the camera frame.
    Eigen::Matrix4f rotate = Eigen::Matrix4f::Identity();
    // Fill in rotate matrix elements here

    // TODO 5: Construct the translation matrix.
    // Hint: The camera is moved to the origin by translating by -eye.
    Eigen::Matrix4f translate = Eigen::Matrix4f::Identity();
    // Fill in translate matrix elements here

    // TODO 6: Combine rotation and translation to form the final view matrix.
    // Hint: The order of multiplication matters.
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    // view = ...

    return view;
}

Eigen::Matrix4f get_model_matrix_transformation(float scale,
                                                float rotation_angle,
                                                Eigen::Vector3f translation)
{
    // TODO 1: Construct the scaling matrix.
    // Hint: Scaling should be applied uniformly along x, y, and z.
    Eigen::Matrix4f S = Eigen::Matrix4f::Identity();
    // Fill in the scaling matrix elements

    // TODO 2: Convert the rotation angle from degrees to radians.
    // Hint: Trigonometric functions expect radians.
    // float radian_angle = ...

    // TODO 3: Construct the rotation matrix around the Z axis.
    // Hint: Use cos(theta) and sin(theta).
    Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
    // Fill in the rotation matrix elements

    // TODO 4: Construct the translation matrix.
    // Hint: Translation moves the object in world space.
    Eigen::Matrix4f T = Eigen::Matrix4f::Identity();
    // Fill in the translation matrix elements

    // TODO 5: Combine the transformation matrices.
    // Experiment with different multiplication orders mentioned in the assignment , such as:
    // T * R * S, S * R * T, S * T * R,...
    // Observe how the order affects the final transformation.
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    // model = ...

    return model;
}
