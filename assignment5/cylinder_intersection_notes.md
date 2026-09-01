# Cylinder Ray Intersection — Problems & Solutions

## Overview

A ray-traced cylinder (axis-aligned along Y) is composed of three surfaces:
- **Side surface** — the curved lateral wall
- **Top cap** — flat disk at `y = center.y + halfHeight`
- **Bottom cap** — flat disk at `y = center.y - halfHeight`

Each surface requires its own intersection test. Three bugs caused visible holes in the rendered output.

---

## Problem 1: Seam Holes at Cap Edges

### What happened
The side surface intersection used **strict inequalities** to clamp the hit point within the cylinder's height:

```cpp
// BUGGY
if (y > -halfHeight && y < halfHeight)
```

When a ray hits exactly at the boundary (`y == ±halfHeight`), floating-point imprecision causes:
- The **side test** to reject it (`y` is not strictly inside the range)
- The **cap disk test** to also reject it (the hit point on the cap plane lands just outside `radius²`)

Both tests miss, so the ray returns the background color — a visible hole along the seam ring between the side and caps.

### Fix
Use **inclusive bounds** on the side surface height check:

```cpp
// FIXED
if (y >= -halfHeight && y <= halfHeight)
```

Boundary rays are now accepted by the side test, and since `tBest` tracks the nearest hit, there is no double-counting with the caps.

---

## Problem 2: Shadow Acne (Self-Intersection on Shadow Rays)

### What happened
After computing a hit point on the cylinder, the renderer casts shadow rays toward each light. The shadow ray origin is offset from the surface by a small epsilon:

```cpp
Vector3f shadowPointOrig = (dotProduct(dir, N) < 0)
    ? hitPoint + N * scene.epsilon   // scene.epsilon = 0.00001
    : hitPoint - N * scene.epsilon;
```

The intersection test used `t > 0.0f` as its only minimum threshold:

```cpp
// BUGGY
if (t > 0.0f && t < tBest)
```

With `scene.epsilon = 0.00001`, the offset is often smaller than floating-point noise in the hit point computation. The shadow ray origin ends up **still on (or just inside) the cylinder surface**, so the cylinder's own intersection returns a valid hit at `t ≈ 1e-7`. The point falsely appears to be in shadow.

When both lights are falsely occluded this way, `lightAmt = 0` and the pixel goes near-black — appearing as a hole.

### Fix
Enforce a **minimum t threshold** (`tMin = 1e-4f`) in all intersection tests:

```cpp
const float tMin = 1e-4f;

// FIXED
if (t > tMin && t < tBest)
```

This rejects any intersection closer than `1e-4` units, which is far below any legitimate hit distance in the scene but large enough to clear floating-point surface noise.

---

## Problem 3: Near-Vertical Ray Instability

### What happened
The side surface intersection solves a quadratic in the XZ plane only:

```
a = dir.x² + dir.z²
b = 2 * (oc.x·dir.x + oc.z·dir.z)
c = oc.x² + oc.z² - radius²
```

When the ray direction is nearly parallel to the Y axis (`dir.x ≈ 0, dir.z ≈ 0`), `a ≈ 0`. Inside `solveQuadratic`, this causes a **division by zero** or produces astronomically large, meaningless roots. Those garbage `t` values can pass the height-range check and corrupt `tBest`, causing the cap intersection to be skipped and the wrong surface (or nothing) to be reported.

### Fix
Guard the quadratic with an `a` threshold before solving:

```cpp
// FIXED
if (a > 1e-8f) {
    // solve quadratic for side surface
}
```

For near-vertical rays, the side surface cannot be hit anyway — the ray will hit a cap (handled separately) or miss entirely. Skipping the ill-conditioned quadratic is both correct and numerically safe.

---

## Summary Table

| # | Problem | Root Cause | Fix |
|---|---------|------------|-----|
| 1 | Seam holes at cap/side boundary | Strict `>` / `<` on height bounds | Use `>=` / `<=` |
| 2 | Dark patches (shadow acne) | `t > 0` allows self-hit at `t ≈ 1e-7` | Use `t > 1e-4` minimum threshold |
| 3 | Random holes from near-vertical rays | `a ≈ 0` causes division-by-zero in quadratic | Guard with `if (a > 1e-8f)` |
