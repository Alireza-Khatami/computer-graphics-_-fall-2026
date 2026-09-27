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
        // Get the texture value at the texture coordinates
        float u = std::min(1.0f, std::max(0.0f, payload.tex_coords.x()));
        float v = std::min(1.0f, std::max(0.0f, payload.tex_coords.y()));
        return_color = payload.texture->getColorBilinear(u, v);
    }
    else {
        // If there is no texture, use white as default
        return_color = Eigen::Vector3f(255, 255, 255);
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

    Eigen::Vector3f result_color = ka.cwiseProduct(amb_light_intensity);

    Eigen::Vector3f view_dir = (eye_pos - point).normalized();

    for (auto& light : lights) {
        Eigen::Vector3f light_dir = (light.position - point).normalized();
        float r2 = (light.position - point).squaredNorm();

        // Diffuse
        float diff = std::max(0.0f, normal.dot(light_dir));
        Eigen::Vector3f diffuse = (kd.cwiseProduct(light.intensity / r2)) * diff;

        // Specular
        Eigen::Vector3f half_vec = (view_dir + light_dir).normalized();
        float spec = pow(std::max(0.0f, normal.dot(half_vec)), p);
        Eigen::Vector3f specular = (ks.cwiseProduct(light.intensity / r2)) * spec;

        result_color += diffuse + specular;
    }

    return result_color * 255.f;
}
