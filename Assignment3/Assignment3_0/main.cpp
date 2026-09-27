
#include <iostream>
#include <opencv2/opencv.hpp>

#include "OBJ_Loader.h"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Triangle.hpp"
#include "global.hpp"
#include "rasterizer.hpp"
#include "main.h"

// LookAt view matrix: places the camera at `eye_pos`, always facing the
// origin (where the model sits), with +Y as up.
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
  Eigen::Vector3f target(0, 0, 0);
  Eigen::Vector3f up(0, 1, 0);
  Eigen::Vector3f z_axis = (eye_pos - target).normalized();  // target -> eye
  Eigen::Vector3f x_axis = up.cross(z_axis).normalized();
  Eigen::Vector3f y_axis = z_axis.cross(x_axis);

  Eigen::Matrix4f view;
  view << x_axis.x(), x_axis.y(), x_axis.z(), -x_axis.dot(eye_pos),
          y_axis.x(), y_axis.y(), y_axis.z(), -y_axis.dot(eye_pos),
          z_axis.x(), z_axis.y(), z_axis.z(), -z_axis.dot(eye_pos),
          0,          0,          0,           1;
  return view;
}

// Camera position on a sphere of the given radius around the origin.
// yaw orbits around the Y axis (a/d), pitch tilts up/down (w/s).
// yaw = pitch = 0 gives (0, 0, radius), the original fixed camera.
Eigen::Vector3f orbit_eye_pos(float radius, float yaw_deg, float pitch_deg) {
  float yaw = yaw_deg * MY_PI / 180.f;
  float pitch = pitch_deg * MY_PI / 180.f;
  return Eigen::Vector3f(radius * cos(pitch) * sin(yaw),
                         radius * sin(pitch),
                         radius * cos(pitch) * cos(yaw));
}

Eigen::Matrix4f get_model_matrix(float angle) {
  Eigen::Matrix4f rotation;
  angle = angle * MY_PI / 180.f;
  rotation << cos(angle), 0, sin(angle), 0, 0, 1, 0, 0, -sin(angle), 0,
      cos(angle), 0, 0, 0, 0, 1;

  Eigen::Matrix4f scale;
  scale << 2.5, 0, 0, 0, 0, 2.5, 0, 0, 0, 0, 2.5, 0, 0, 0, 0, 1;

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1;

  return translate * rotation * scale;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar) {
  // TODO: Use the same projection matrix from the previous assignments.
  // TODO: Implement this function
  // Create the projection matrix for the given parameters.
  // Then return it.    

  float radFOV = DEG2RAD(eye_fov);

  float t = abs(zNear) * tan(radFOV / 2.0f);
  float r = t * aspect_ratio;

  float A = zNear + zFar;
  float B = -zNear * zFar;

  // Persepctive-to-orthographic matrix
  Eigen::Matrix4f persp_to_ortho;
  persp_to_ortho << -zNear, 0, 0, 0,
      0, -zNear, 0, 0,
      0, 0, A, B,
      0, 0, 1, 0;

  Eigen::Matrix4f ortho_scale;
  float left = -r;
  float bottom = -t;
  ortho_scale << (2 / (r - left)), 0, 0, 0,
      0, (2 / (t - bottom)), 0, 0,
      0, 0, (2 / (zNear - zFar)), 0,
      0, 0, 0, 1;

  Eigen::Matrix4f ortho_trans;
  ortho_trans << 1, 0, 0, (-(r + left) / 2),
      0, 1, 0, (-(t + bottom) / 2),
      0, 0, 1, (-(zNear + zFar) / 2),
      0, 0, 0, 1;

  return (ortho_scale * ortho_trans) * persp_to_ortho;
        // Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

        // // Convert FOV from degrees to radians
        // float rad_fov = eye_fov * MY_PI / 180.0f;

        // // Compute height and width of the near plane
        // float t = std::tan(rad_fov / 2.0f) * std::abs(zNear);
        // float r = t * aspect_ratio;
        // float l = -r, b = -t;

        // // --------------------- Perspective Projection ---------------------
        // Eigen::Matrix4f persp_to_ortho;
        // persp_to_ortho <<
        //     -zNear, 0, 0, 0,
        //     0, -zNear, 0, 0,
        //     0, 0, zNear + zFar, -zNear * zFar,
        //     0, 0, 1, 0;

        // // --------------------- Orthographic Projection ---------------------
        // Eigen::Matrix4f ortho;
        // ortho <<
        //     2/ (r - l), 0, 0, -(r + l) / (r - l),
        //     0, 2 / (t - b), 0, -(t + b) / (t - b),
        //     0, 0, -2 / ( zFar - zNear  ), -(zNear + zFar) / (zFar -zNear  ),
        //     0, 0, 0, 1;

        // // Combine perspective-to-orthographic and orthographic projection
        // projection = ortho * persp_to_ortho;

        // return projection;
    
}

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

struct light {
  Eigen::Vector3f position;
  Eigen::Vector3f intensity;
};

// View matrix of the current frame. The fragment shader receives view-space
// positions and normals, so the world-space lights are moved into view space
// with this matrix, and the eye sits at the view-space origin.
static Eigen::Matrix4f g_view = Eigen::Matrix4f::Identity();

static Eigen::Vector3f to_view_space(const Eigen::Vector3f& world_pos) {
  return (g_view * Eigen::Vector4f(world_pos.x(), world_pos.y(),
                                   world_pos.z(), 1.0f)).head<3>();
}

// Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload) {
//     Eigen::Vector3f return_color = { 0, 0, 0 };

//     if (payload.texture) {
//         // Get the texture value at the texture coordinates of the current fragment
//         return_color = payload.texture->getColor(payload.tex_coords.x(), payload.tex_coords.y());
//         //return_color = { payload.tex_coords.x()*255, payload.tex_coords.y()*255 ,1};
//     }
//     Eigen::Vector3f texture_color;
//     texture_color << return_color.x(), return_color.y(), return_color.z();
//     //texture_color << 0, 255, 0;
//     // Phong reflection model coefficients
//     Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005); // Ambient reflectivity
//     Eigen::Vector3f kd = texture_color / 255.f;  // Diffuse reflectivity from texture
//     Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937); // Specular reflectivity


//     //Vector4f prj_m = get_projection_matrix(90, 1, 0.1, 50);

//      //Light sources
//     auto l1 = light{ {20, 20, 20}, {500, 500, 500} };
//     auto l2 = light{ {-20, 20, 0}, {500, 500, 500} };

//     std::vector<light> lights = { l1, l2 };

//     Eigen::Vector3f amb_light_intensity{ 10, 10, 10 }; // Ambient light intensity
//     Eigen::Vector3f eye_pos{ 0, 0, 10 };  // Camera position
//     float p = 150;  // Shininess factor for specular highlights

//     Eigen::Vector3f color = texture_color;
//     Eigen::Vector3f point = payload.view_pos;
//     Eigen::Vector3f normal = payload.normal.normalized();

//     Eigen::Vector3f result_color = { 0, 0, 0 };
//     Eigen::Vector3f view_dir = (eye_pos - point).normalized();

//     for (auto& light : lights) {
//         Eigen::Vector3f light_dir = (light.position - point).normalized();
//         float distance_squared = (light.position - point).squaredNorm();
//         Eigen::Vector3f intensity = light.intensity / distance_squared;  // Attenuation

//         //  Ambient Component 
//         Eigen::Vector3f ambient = ka.cwiseProduct(amb_light_intensity);

//         //  Diffuse Component (Lambertian shading) 
//         float diff = std::max(normal.dot(light_dir), 0.0f);
//         Eigen::Vector3f diffuse = kd.cwiseProduct(intensity) * diff;

//         //  Specular Component (Blinn-Phong model)
//         Eigen::Vector3f half_vector = (view_dir + light_dir).normalized();
//         float spec = std::pow(std::max(normal.dot(half_vector), 0.0f), p);
//         Eigen::Vector3f specular = ks.cwiseProduct(intensity) * spec;

//         // Accumulate lighting contributions
//         result_color += ambient + diffuse + specular;
//     }

//     return result_color * 255.f; // Convert to RGB range
// }



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

    // Lights are fixed in world space; convert them to view space so they
    // match `point` and `normal` (both view space).
    auto l1 = light{ to_view_space({20, 20, 20}), {500, 500, 500} };
    auto l2 = light{ to_view_space({-20, 20, 0}), {500, 500, 500} };

    std::vector<light> lights = { l1, l2 };
    Eigen::Vector3f amb_light_intensity{ 10, 10, 10 };
    Eigen::Vector3f eye_pos{ 0, 0, 0 };  // the camera is the view-space origin

    float p = 150;

    Eigen::Vector3f color = texture_color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal.normalized();

    Eigen::Vector3f result_color = ka.cwiseProduct(amb_light_intensity);

    Vector3f view_dir = (eye_pos - point).normalized();

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


int main(int argc, const char** argv) {
  std::vector<Triangle*> TriangleList;
  float angle = 140.0;
  bool command_line = false;
  std::string filename ;
  objl::Loader Loader;
  std::string obj_path ;
  rst::rasterizer r(700, 700);
  std::string texture_path = "spot_texture.png";
  // auto texture_path = std::string(argv[2]);//"hmap.jpg";
  std::function<Eigen::Vector3f(fragment_shader_payload)> active_shader =texture_fragment_shader;
  r.set_vertex_shader(vertex_shader);
  r.set_fragment_shader(active_shader);
  Eigen::Vector3f eye_pos = {0, 0, 10};
  if (argc >=2) command_line = true;
  if (argc == 3) {
    filename = std::string(argv[1]);
    obj_path = std::string(argv[2]);
        std::cout << "Rasterizing using the normal shader\n";
        active_shader = normal_fragment_shader;
        r.set_fragment_shader(active_shader);
      }
      else if (argc == 4 ) {
        filename = std::string(argv[1]);
        obj_path = std::string(argv[2]);
        texture_path = std::string(argv[3]);
        std::cout << "Rasterizing using the texture shader\n";
        active_shader = texture_fragment_shader;
        r.set_fragment_shader(active_shader);
        r.set_texture(Texture( texture_path));
    }
    
  if (command_line) {
    read_obj_file(Loader, obj_path, TriangleList);
    std::cout<< "Rasterizing using commandline model\n";
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    g_view = get_view_matrix(eye_pos);
    r.set_model(get_model_matrix(angle));
    r.set_view(g_view);
    r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

    r.draw(TriangleList);
    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
    if (r.texture) {
        bool is_using_bilinear = r.texture->is_using_bilinear;
        if(is_using_bilinear)
            std::cout << "Bilinear interpolation is used for texture sampling.\n";
        else
            std::cout << "Nearest neighbor sampling is used for texture sampling.\n";
    }
    cv::imwrite(filename, image);
    
    return 0;
  }
  
  // Interactive mode (no arguments): default model and texture, paths are
  // relative to the build folder the executable runs from.
  filename = "output.png";
  obj_path = "../models/spot/spot_triangulated_good.obj";
  texture_path = "../models/spot/spot_texture.png";
  read_obj_file(Loader, obj_path, TriangleList);
  r.set_texture(Texture(texture_path));

  // Orbit camera around the model (same controls as Assignment 2).
  const float cam_radius = eye_pos.norm();
  float cam_yaw = 0.0f;
  float cam_pitch = 0.0f;

  std::cout << "\n-- Controls --\n"
            << "  a / d : orbit camera left / right\n"
            << "  w / s : orbit camera up / down\n"
            << "  Esc/q : quit\n";

  int key = 0;
  while (key != 27 && key != 'q') {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);

    g_view = get_view_matrix(orbit_eye_pos(cam_radius, cam_yaw, cam_pitch));
    r.set_model(get_model_matrix(angle));
    r.set_view(g_view);
    r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

    r.draw(TriangleList);
    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

    cv::imshow("image", image);
    cv::imwrite(filename, image);
    key = cv::waitKey(0);

    if (key == 'a') {
      cam_yaw -= 10;
    } else if (key == 'd') {
      cam_yaw += 10;
    } else if (key == 'w') {
      // Stay short of the poles, where LookAt's up vector becomes degenerate.
      cam_pitch = std::min(cam_pitch + 10, 80.0f);
    } else if (key == 's') {
      cam_pitch = std::max(cam_pitch - 10, -80.0f);
    }
  }
  return 0;
}

void read_obj_file(objl::Loader &Loader, std::string &obj_path, std::vector<Triangle *> &TriangleList)
{
  bool loadout = Loader.LoadFile(obj_path);
  for (auto mesh : Loader.LoadedMeshes)
  {
    for (int i = 0; i < mesh.Vertices.size(); i += 3)
    {
      Triangle *t = new Triangle();
      for (int j = 0; j < 3; j++)
      {
        t->setVertex(j, Vector4f(mesh.Vertices[i + j].Position.X,
                                 mesh.Vertices[i + j].Position.Y,
                                 mesh.Vertices[i + j].Position.Z, 1.0));
        t->setNormal(j, Vector3f(mesh.Vertices[i + j].Normal.X,
                                 mesh.Vertices[i + j].Normal.Y,
                                 mesh.Vertices[i + j].Normal.Z));
        t->setTexCoord(j, Vector2f(mesh.Vertices[i + j].TextureCoordinate.X,
                                   mesh.Vertices[i + j].TextureCoordinate.Y));
      }
      TriangleList.push_back(t);
    }
  }
}
