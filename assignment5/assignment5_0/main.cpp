#include "Cylinder.hpp"
#include "Light.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "cube.hpp"
#include "inititiate_scene.hpp"
#include "shadow_target_scene.hpp"
#include <optional>
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
  { 4, "shadow_target_scene",     "shadow_target_scene.ppm"     },
};

// Build a scene for the given mode, add the shared floor and lights, then render.
static void build_and_render(int mode, const std::string& filename) {
  Scene scene(1280, 960);

  switch (mode) {
    case 0: instantiate_main_scene(scene);            break;
    case 1: instantiate_sphere_cube_scene(scene);     break;
    case 2: instantiate_sphere_material_scene(scene); break;
    case 3: instantiate_sphere_cylinder_scene(scene); break;
    case 4: instantiate_shadow_target_scene(scene);   break;
    default: break;
  }

  // Shared floor mesh
  Vector3f verts[4] = {{-15, -3, -5}, {15, -3, -5}, {15, -3, -19}, {-15, -3, -19}};
  uint32_t vertIndex[6] = {0, 1, 3, 1, 2, 3};
  Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
  auto mesh = std::make_unique<MeshTriangle>(verts, vertIndex, 2, st);
  mesh->materialType = DIFFUSE_AND_GLOSSY;
  scene.Add(std::move(mesh));

  // Shared lights (the shadow target scene has its own single light)
  if (mode != 4) {
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 0.5));
    scene.Add(std::make_unique<Light>(Vector3f(30, 50, -12), 0.5));
    scene.Add(std::make_unique<Light>(Vector3f(0, 0, 5), 0.5));
  } else {
    std::cout << "Light at (" << kLightFoot.x << ", " << kLightY << ", "
              << kLightFoot.z << "), marker at (" << kMarker.x << ", "
              << kMarker.y << ", " << kMarker.z << ")\n";
  }

  Renderer r;
  r.Render(scene, filename);
  if (mode == 4) check_shadow_target(scene);
}

// TA only: print the analytic light height for the shadow target scene and
// check it by tracing shadow rays from floor points against the pole.
std::optional<hit_payload> trace(const Vector3f& orig, const Vector3f& dir,
                                 const std::vector<std::unique_ptr<Object> >& objects);

// Distance from the light foot to the shadow tip, measured along the line
// from the light foot through the marker.
static float shadow_tip(float lightY) {
  Scene scene(1, 1);
  instantiate_shadow_target_scene(scene, lightY);
  const Object& pole = *scene.get_objects()[0];
  Vector3f light(kLightFoot.x, lightY, kLightFoot.z);
  Vector3f dirFloor = normalize(kMarker - kLightFoot);

  auto inShadow = [&](float u) {
    Vector3f p = kLightFoot + dirFloor * u;
    Vector3f toLight = light - p;
    float dist2 = dotProduct(toLight, toLight);
    float t = kInfinity;
    uint32_t idx;
    Vector2f uv;
    return pole.intersect(p, normalize(toLight), t, idx, uv) && t * t < dist2;
  };

  // Shadowed just behind the pole, lit far away: bisect for the boundary.
  Vector3f footToPole = kPoleBase - kLightFoot;
  float lo = std::sqrt(dotProduct(footToPole, footToPole)) + kPoleRadius + 1e-3f;
  float hi = 100.0f;
  if (!inShadow(lo)) return lo;
  for (int i = 0; i < 60; ++i) {
    float mid = 0.5f * (lo + hi);
    (inShadow(mid) ? lo : hi) = mid;
  }
  return lo;
}

static void solve() {
  Vector3f footToPole = kPoleBase - kLightFoot;
  Vector3f footToMarker = kMarker - kLightFoot;
  float D = std::sqrt(dotProduct(footToPole, footToPole));
  float s = std::sqrt(dotProduct(footToMarker, footToMarker));
  float h = kPoleHeight, r = kPoleRadius;
  float H = h * s / (s - (D + r));
  float floorY = kLightFoot.y;
  std::cout << "D = " << D << ", s = " << s << ", h = " << h << ", r = " << r << "\n"
            << "collinearity |cross| = "
            << std::fabs(footToPole.x * footToMarker.z - footToPole.z * footToMarker.x) << "\n"
            << "analytic: H = h*s/(s-(D+r)) = " << H << ", light y = " << floorY + H
            << "   (kLightY = " << kLightY << ")\n";

  // Brute force: bisect the light height until the traced shadow tip hits s.
  float lo = floorY + h + 0.05f, hi = floorY + 100.0f;  // tip shrinks as the light rises
  for (int i = 0; i < 60; ++i) {
    float mid = 0.5f * (lo + hi);
    (shadow_tip(mid) > s ? lo : hi) = mid;
  }
  std::cout << "traced:   light y = " << 0.5f * (lo + hi) << "\n";
  for (float y : {kLightY, 9.0f, floorY + h * s / (s - D), floorY + H + 3.0f})
    std::cout << "  light y = " << y << " -> shadow tip at " << shadow_tip(y)
              << " (target " << s << ")\n";
}

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv) {
  if (argc > 3 || argc == 1) {
    std::cout << "Usage: ./RayTracing <scene_mode|validate> [output.ppm]" << std::endl;
    return 0;
  }

  std::string arg = argv[1];

  if (arg == "solve") {
    solve();
    return 0;
  }

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

  // Normal mode: render a single scene to binary.ppm (or the given file)
  int scene_mode = arg.empty() ? 0 : std::stoi(arg);
  build_and_render(scene_mode, argc == 3 ? argv[2] : "binary.ppm");

  return 0;
}
