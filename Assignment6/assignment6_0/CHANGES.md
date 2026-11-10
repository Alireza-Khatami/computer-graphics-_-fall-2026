# Glass Material — Changes Summary

## Overview

Added a `GLASS` material type that supports Fresnel-based reflection and refraction (delta BSDF). The scene now renders two variants: the original diffuse Cornell Box and a version with a glass tall box.

---

## `Material.hpp`

### 1. New enum value
```cpp
// Before
enum MaterialType { DIFFUSE };

// After
enum MaterialType { DIFFUSE, GLASS };
```

### 2. New method declaration
```cpp
inline bool isSpecular();
```
Returns `true` when the material type is `GLASS`. Used by `castRay` to take the specular code path.

### 3. New method definition
```cpp
bool Material::isSpecular() { return m_type == GLASS; }
```

### 4. `sample()` — added GLASS case
Computes the Fresnel reflectance `kr` for the incident direction, then stochastically returns either the reflected or refracted direction. Falls back to reflection on total internal reflection.
```cpp
case GLASS: {
    float kr;
    fresnel(-wi, N, ior, kr);          // wi is outgoing; -wi is incident
    if (get_random_float() < kr)
        return reflect(-wi, N).normalized();
    else {
        Vector3f refracted = refract(-wi, N, ior);
        if (refracted.noletrm() < EPSILON) return reflect(-wi, N).normalized();
        return refracted.normalized();
    }
}
```
Note: `reflect()` and `refract()` and `fresnel()` were already private helpers in `Material.hpp` — they are now used here for the first time.

### 5. `pdf()` — added GLASS case
```cpp
case GLASS: { return 1.0f; }
```
Delta distributions have pdf = 1; the Fresnel weight is baked into the stochastic sampling in `sample()`.

### 6. `eval()` — added GLASS case
```cpp
case GLASS: { return Vector3f(1.0f); }
```
Returns 1 because the energy bookkeeping is handled in `castRay` (no cosine division needed for specular paths).

---

## `Scene.cpp`

### `castRay()` — added specular branch

Inserted before the diffuse direct/indirect lighting block. For specular materials:
- Skips direct lighting entirely (delta BSDFs cannot be sampled by area light sampling).
- Samples one specular ray via `hit.m->sample()`.
- Offsets the ray origin along/against the normal to avoid self-intersection (sign depends on whether the ray is reflected or refracted).
- Recurses and divides by `RussianRoulette`.

```cpp
if (hit.m->isSpecular()) {
    if (get_random_float() >= RussianRoulette) return Vector3f(0.0f);
    Vector3f wi = hit.m->sample(wo, N);
    Vector3f origin = (dotProduct(wi, N) > 0)
        ? hitPoint + N * EPSILON
        : hitPoint - N * EPSILON;
    Ray newRay(origin, wi);
    return castRay(newRay) / RussianRoulette;
}
```

Also removed three duplicate variable declarations (`hitPoint`, `N`, `wo`) that were left over from the original code after hoisting them above the new specular block.

---

## `Renderer.hpp`

Added `filename` parameter (with default `"binary.ppm"`) to `Render()` so different scenes can save to different output files.

```cpp
// Before
void Render(const Scene& scene, int spp);

// After
void Render(const Scene& scene, int spp, const std::string& filename = "binary.ppm");
```

Also added `#include <string>` and moved `#pragma once` to the top.

---

## `Renderer.cpp`

Updated the `fopen` call to use the `filename` parameter instead of the hardcoded string.

```cpp
// Before
FILE* fp = fopen("binary.ppm", "wb");

// After
FILE* fp = fopen(filename.c_str(), "wb");
```

---

## `main.cpp`

Replaced the single scene with two scoped scene blocks rendered sequentially.

| Scene | Output file | Tall box | Right wall |
|-------|-------------|----------|------------|
| Scene 1 (commented out) | `binary_diffuse.ppm` | white diffuse | green diffuse |
| Scene 2 | `binary_glass.ppm` | **glass (ior 1.5)** | green diffuse |

Key additions in Scene 2:
```cpp
Material* glass_box = new Material(GLASS, Vector3f(0.0f));
glass_box->ior = 1.5f;   // crown glass
...
MeshTriangle tallbox("../models/cornellbox/tallbox.obj", glass_box);
```

Each scene is wrapped in its own `{}` block so that stack-allocated `MeshTriangle` and heap `Material*` objects are scoped to that render, and the `Renderer r` is shared between both.
