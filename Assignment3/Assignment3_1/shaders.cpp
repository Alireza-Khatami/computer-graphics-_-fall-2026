#include "shaders.hpp"

#include <algorithm>
#include <cmath>

#include "light.hpp"

Eigen::Vector3f vertex_shader(const vertex_shader_payload& payload) {
  return payload.position;
}

Eigen::Vector3f normal_fragment_shader(const fragment_shader_payload& payload) {
  Eigen::Vector3f return_color = (payload.normal.head<3>().normalized() +
                                  Eigen::Vector3f(1.0f, 1.0f, 1.0f)) /
                                 2.f;
  Eigen::Vector3f result;
  result << return_color.x() * 255, return_color.y() * 255,
      return_color.z() * 255;
  return result;
}

static Eigen::Vector3f reflect(const Eigen::Vector3f& vec,
                               const Eigen::Vector3f& axis) {
  auto costheta = vec.dot(axis);
  return (2 * costheta * axis - vec).normalized();
}

Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload) {
    Eigen::Vector3f return_color = { 0, 0, 0 };
    if (payload.texture) {
        // TODO:
        // Get the texture value at the texture coordinates of the current fragment
        // using Texture::getColorBilinear() (not getColor()).
    }
    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005); // Ambient reflectivity
    Eigen::Vector3f kd = texture_color / 255.f;  // Diffuse reflectivity from texture
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937); // Specular reflectivity

    // Lights are fixed in world space; view_space_lights() has them moved
    // into view space so they match `point` and `normal` (both view space).
    const std::vector<light>& lights = view_space_lights();
    Eigen::Vector3f amb_light_intensity = ambient_light_intensity();
    Eigen::Vector3f eye_pos{ 0, 0, 0 };  // the camera is the view-space origin

    float p = 150;

    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal.normalized();

    Eigen::Vector3f result_color = { 0, 0, 0 };
    Eigen::Vector3f view_dir = (eye_pos - point).normalized();
    // TODO:
    // 1. Add the *ambient* component (ka and amb_light_intensity) to
    //    *result_color* once; it does not depend on the lights below.
    for (auto& light : lights) {
        // TODO:
        // 2. For each light source, calculate the *diffuse* and *specular*
        //    (Blinn-Phong) components.
        // 3. Then, accumulate the results on the *result_color* variable.
    }

    return result_color * 255.f;
}

Eigen::Vector3f diffuse_fragment_shader(const fragment_shader_payload& payload) {
    Eigen::Vector3f texture_color = { 255, 255, 255 };
    if (payload.texture) {
        float u = std::min(1.0f, std::max(0.0f, payload.tex_coords.x()));
        float v = std::min(1.0f, std::max(0.0f, payload.tex_coords.y()));
        texture_color = payload.texture->getColorBilinear(u, v);
    }

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = texture_color / 255.f;

    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal.normalized();

    Eigen::Vector3f result_color = ka.cwiseProduct(ambient_light_intensity());
    for (auto& light : view_space_lights()) {
        Eigen::Vector3f light_dir = (light.position - point).normalized();
        float r2 = (light.position - point).squaredNorm();
        float diff = std::max(0.0f, normal.dot(light_dir));
        result_color += kd.cwiseProduct(light.intensity / r2) * diff;
    }

    return result_color * 255.f;
}
