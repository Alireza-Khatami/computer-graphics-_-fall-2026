#pragma once

#include "Cylinder.hpp"
#include "Light.hpp"
#include "Scene.hpp"

// Shadow target scene (./RayTracing 4)
//
// A pole stands on the floor (y = -3) and a single point light hangs straight
// above the point kLightFoot. The white disk on the floor marks the target:
// the tip of the pole's shadow must land exactly on its centre.

const Vector3f kPoleBase(-2.0f, -3.0f, -9.0f);    // centre of the pole's bottom cap (on the floor)
const float kPoleRadius = 0.5f;
const float kPoleHeight = 3.0f;
const Vector3f kLightFoot(-6.0f, -3.0f, -6.0f);   // point on the floor straight below the light
const Vector3f kMarker(1.2f, -3.0f, -11.4f);      // centre of the target marker (on the floor)

// TODO ---------------------------------------------------------------
// y coordinate of the light. Work out by hand the value for which the tip of
// the pole's shadow lands on the centre of the marker, and set it here.
// (See "Shadow Target Scene" in the handout. Do not find it by trial and error:
// the report must show how you derived it.)
// --------------------------------------------------------------------
const float kLightY = 9.0f;

void instantiate_shadow_target_scene(Scene& scene, float lightY = kLightY) {
  auto pole = std::make_unique<Cylinder>(
      kPoleBase + Vector3f(0, kPoleHeight / 2, 0), kPoleRadius, kPoleHeight);
  pole->materialType = DIFFUSE_AND_GLOSSY;
  pole->Kd = 0.8f;
  pole->Ks = 0.5f;
  pole->specularExponent = 32.0f;
  scene.Add(std::move(pole));

  // Thin white disk lying on the floor
  auto marker = std::make_unique<Cylinder>(kMarker + Vector3f(0, 0.005f, 0), 0.35f, 0.01f);
  marker->checkered = false;
  marker->diffuseColor = Vector3f(1.0f, 1.0f, 1.0f);
  scene.Add(std::move(marker));

  // The only light in this scene
  scene.Add(std::make_unique<Light>(Vector3f(kLightFoot.x, lightY, kLightFoot.z), 1.0));
}

// Prints whether the tip of the pole's shadow is on the marker centre. Two floor
// points on the line from the light foot through the marker are tested, one just
// before the centre and one just past it: the first must be in the pole's
// shadow and the second must be lit.
void check_shadow_target(const Scene& scene) {
  const float tolerance = 0.02f;  // about one pixel at the marker
  const Object& pole = *scene.get_objects()[0];  // the pole is added first
  const Vector3f light = scene.get_lights()[0]->position;
  Vector3f along = kMarker - kLightFoot;
  along = normalize(Vector3f(along.x, 0, along.z));

  auto inShadow = [&](const Vector3f& p) {
    Vector3f toLight = light - p;
    float dist2 = dotProduct(toLight, toLight);
    float t = kInfinity;
    uint32_t index;
    Vector2f uv;
    return pole.intersect(p, normalize(toLight), t, index, uv) && t * t < dist2;
  };
  bool before = inShadow(kMarker - along * tolerance);
  bool after = inShadow(kMarker + along * tolerance);

  std::cout << "Shadow target check: ";
  if (before && !after)
    std::cout << "YES - the shadow tip is on the centre of the marker\n";
  else if (!before && !after)
    std::cout << "NO - the shadow stops before the centre of the marker\n";
  else
    std::cout << "NO - the shadow goes past the centre of the marker\n";
}
