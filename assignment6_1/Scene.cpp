#include "Scene.hpp"

#include <cassert>

void Scene::buildBVH() {
  printf(" - Generating BVH...\n\n");
  this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const {
  return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const {
  float emit_area_sum = 0;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    if (objects[k]->hasEmit()) {
      emit_area_sum += objects[k]->getArea();
    }
  }
  float p = get_random_float() * emit_area_sum;  //[0~1]*13650
  emit_area_sum = 0;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    if (objects[k]->hasEmit()) {
      emit_area_sum += objects[k]->getArea();
      if (p <= emit_area_sum) {  // random get the first area > p light,return
        objects[k]->Sample(pos, pdf);
        break;
      }
    }
  }
}

bool Scene::trace(const Ray &ray, const std::vector<Object *> &objects,
                  float &tNear, uint32_t &index, Object **hitObject) {
  *hitObject = nullptr;
  for (uint32_t k = 0; k < objects.size(); ++k) {
    float tNearK = kInfinity;
    uint32_t indexK;
    Vector2f uvK;
    if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
      *hitObject = objects[k];
      tNear = tNearK;
      index = indexK;
    }
  }

  return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray) const {
  // TO DO Implement Path Tracing Algorithm here
   // TODO: Implement intersect(ray) to return hit info or background
// TODO: Return emission immediately if the hit surface is a light

// TODO: Implement sampleLight() to sample a point on emissive surfaces and get pdf_light
// TODO: Cast a shadow ray toward the sampled light and check if the light is visible
// TODO: Compute BRDF, cosine terms, distance^2, and accumulate direct lighting

// TODO: Apply Russian Roulette to decide whether to continue
// TODO: Sample an indirect direction from the material
// TODO: Cast a secondary ray and continue only if it hits a non-emissive object
// TODO: Evaluate BRDF, compute pdf, cosine term, and accumulate indirect lighting

// TODO: Return L_dir + L_indir
return Vector3f(0.0f);
}