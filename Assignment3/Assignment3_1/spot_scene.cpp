#include "spot_scene.hpp"

#include <iostream>

// OBJ_Loader.h defines non-inline functions, so it may only be included in
// this one source file.
#include "OBJ_Loader.h"
#include "light.hpp"
#include "shaders.hpp"
#include "transforms.hpp"

namespace {

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

}  // namespace

SpotScene::SpotScene(const std::string& obj_path,
                     const std::string& texture_path)
    : shader_(texture_fragment_shader) {
  if (!texture_path.empty()) texture_.emplace(texture_path);
  objl::Loader loader;
  std::string path = obj_path;
  read_obj_file(loader, path, triangles_);
}

void SpotScene::render(rst::rasterizer& r) {
  Eigen::Matrix4f view = camera_.view();
  set_world_lights(default_world_lights());
  set_light_view_matrix(view);

  if (texture_) r.set_texture(*texture_);
  r.set_fragment_shader(shader_);
  r.set_model(get_model_matrix(angle_));
  r.set_view(view);
  r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));
  r.draw(triangles_);
}

void SpotScene::print_controls() const {
  std::cout << "\n-- Spot scene --\n"
            << "  a / d : orbit camera left / right\n"
            << "  w / s : orbit camera up / down\n"
            << "  TAB   : switch scene\n"
            << "  Esc/q : quit\n";
}
