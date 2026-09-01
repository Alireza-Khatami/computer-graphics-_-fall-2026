#include "Cylinder.hpp"
#include "Light.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "cube.hpp"
#include "inititiate_scene.hpp"
#include <string>


struct SceneEntry {
  int mode;
  const char* name;
  const char* filename;
};

static const SceneEntry scenes[] = {
  { 0, "main_scene",              "main_scene.ppm"              },
  { 1, "sphere_cube_scene",       "sphere_cube_scene.ppm"       },
  { 2, "sphere_material_scene",   "sphere_material_scene.ppm"   },
  { 3, "sphere_cylinder_scene",   "sphere_cylinder_scene.ppm"   },
};

// Build a scene for the given mode, add the shared floor and lights, then render.
static void build_and_render(int mode, const std::string& filename) {
  Scene scene(1280, 960);

  switch (mode) {
    case 0: instantiate_main_scene(scene);            break;
    case 1: instantiate_sphere_cube_scene(scene);     break;
    case 2: instantiate_sphere_material_scene(scene); break;
    case 3: instantiate_sphere_cylinder_scene(scene); break;
    default: break;
  }

  // Shared floor mesh
  Vector3f verts[4] = {{-15, -3, -5}, {15, -3, -5}, {15, -3, -19}, {-15, -3, -19}};
  uint32_t vertIndex[6] = {0, 1, 3, 1, 2, 3};
  Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
  auto mesh = std::make_unique<MeshTriangle>(verts, vertIndex, 2, st);
  mesh->materialType = DIFFUSE_AND_GLOSSY;
  scene.Add(std::move(mesh));

  // Shared lights
  scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 0.5));
  scene.Add(std::make_unique<Light>(Vector3f(30, 50, -12), 0.5));
  scene.Add(std::make_unique<Light>(Vector3f(0, 0, 5), 0.5));

  Renderer r;
  r.Render(scene, filename);
}

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv) {
  if (argc > 2 || argc == 1) {
    std::cout << "Usage: ./pathtracer <scene_mode|validation>" << std::endl;
    return 0;
  }

  std::string arg = argv[1];

  // Validation mode: render all scenes and save each to a named .ppm file
  if (arg == "validate") {
    for (const auto& s : scenes) {
      std::cout << "\n=== Rendering: " << s.name << " → " << s.filename << " ===\n";
      build_and_render(s.mode, s.filename);
    }
    std::cout << "\nValidation complete. Output files:\n";
    for (const auto& s : scenes)
      std::cout << "  " << s.filename << "\n";
    return 0;
  }

  // Normal mode: render a single scene to binary.ppm
  int scene_mode = arg.empty() ? 0 : std::stoi(arg);
  build_and_render(scene_mode, "binary.ppm");

  return 0;
}

