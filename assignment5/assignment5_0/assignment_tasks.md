# Assignment 5 — Implementation Tasks

This assignment is built on a Whitted-style ray tracer. The codebase handles scene setup, the render loop, and material shading structure. Your job is to fill in four core pieces of the rendering math.

---

## Task 1 — Sphere Ray Intersection (`Sphere.hpp`)

The ray–sphere intersection is solved using the quadratic formula, giving two candidate distances `t0` and `t1` along the ray. The roots are already computed for you via `solveQuadratic`.

**You must determine which root represents the correct hit:**

- A negative `t` means the intersection is *behind* the ray origin — ignore it.
- If both roots are negative, there is no valid intersection → return `false`.
- Otherwise, pick the **smallest positive** `t` and store it in `tnear`.
- Return `true`.

**Relevant variables:** `t0`, `t1`, `tnear`, `hasSolution`

---

## Task 2 — Shadow Test (`Renderer.cpp`)

Inside `castRay`, for diffuse/glossy surfaces, a shadow ray is cast toward each light. The boolean `inShadow` is already computed — it is `true` when another object blocks the path to the light.

**You must gate the diffuse contribution on whether the point is lit:**

- If `inShadow` is `true`, the point receives **no diffuse light** from this source.
- If `inShadow` is `false`, add the normal Lambert term: `light->intensity * LdotN`.
- Accumulate the result into `lightAmt`.

**Relevant variables:** `inShadow`, `lightAmt`, `light->intensity`, `LdotN`

---

## Task 3 — Phong Specular Term (`Renderer.cpp`)

The Phong model has three components: ambient (ignored here), diffuse (Task 2), and specular. The specular term models mirror-like highlights.

**You must compute and accumulate the specular contribution:**

$$
\text{specular} = \left(\max(0,\ -(R \cdot V))\right)^{s} \times I
$$

Where:
- `R` = `reflectionDirection` (reflection of the light direction about the surface normal)
- `V` = `dir` (the incoming view/ray direction)
- `s` = `payload->hit_obj->specularExponent`
- `I` = `light->intensity`

Add this result to `specularColor` for each light in the loop.

**Relevant variables:** `reflectionDirection`, `dir`, `specularExponent`, `light->intensity`, `specularColor`

---

## Task 4 — Cylinder Cap Intersections (`Cylinder.hpp`)

The cylinder's side surface intersection is already implemented. A closed cylinder also needs flat **top and bottom caps** — each is a disk lying on a horizontal plane.

**You must implement the cap intersection tests:**

For each cap (top at `y = +halfHeight`, bottom at `y = -halfHeight`):

1. Find where the ray hits the cap's plane:
$$
t_{cap} = \frac{\pm\,\texttt{halfHeight} - \texttt{oc.y}}{\texttt{dir.y}}
$$
2. Reject if `t_cap <= tMin` (behind origin or self-intersection).
3. Reject if `t_cap >= tBest` (a closer hit already found).
4. Compute the hit point's XZ offset from the cylinder axis:
   `px = oc.x + t_cap * dir.x`,  `pz = oc.z + t_cap * dir.z`
5. Accept only if the point is inside the disk: `px² + pz² <= radius²`
6. If accepted, update `tBest` and set `iBest` to `1` (top) or `2` (bottom).

**Watch out for:**
- Skip the cap test entirely when `dir.y ≈ 0` (ray parallel to the caps — no intersection possible).
- Use `tMin = 1e-4f` as the minimum valid `t` to avoid shadow self-intersection.

**Relevant variables:** `oc`, `dir`, `halfHeight`, `radius2`, `tBest`, `iBest`, `tMin`

---

## Expected Output

Run with scene mode 3 (`./pathtracer 3`) to see two cylinders alongside two glass spheres. A correct implementation shows:

- Solid cylinders with no holes or dark patches
- Smooth shading on the side surface with visible specular highlights
- Hard shadows cast by the cylinders onto the floor
- Glass spheres reflecting and refracting the cylinders behind them
