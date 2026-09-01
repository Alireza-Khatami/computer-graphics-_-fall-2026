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
    Intersection hit = intersect(ray);
    if (!hit.happened)
        return Vector3f(0.0f);  // Background color

    if (hit.m->hasEmission())
        return hit.m->getEmission();  // If hit an emitting surface directly

    Vector3f L_dir(0.0f), L_indir(0.0f);
    Vector3f hitPoint = hit.coords;
    Vector3f N = hit.normal;
    Vector3f wo = -ray.direction;

    // === Direct Lighting ===
    Intersection lightInter;
    float pdf_light = 0.0f;
    sampleLight(lightInter, pdf_light);
    Vector3f x = lightInter.coords;
    Vector3f ws = (x - hitPoint).normalized();
    Vector3f NN = lightInter.normal;
    Vector3f emit = lightInter.emit;

    // Shadow ray
    Ray shadowRay(hitPoint, ws);
    Intersection shadowHit = intersect(shadowRay);
	if (shadowHit.happened && (shadowHit.coords - x).norm() < 1e-3) {//(shadowHit.coords - x).norm() < 1e-2  if the shadow ray hit the light( distance of the ray intesection on light
        Vector3f eval = hit.m->eval(wo, ws, N);
        float dot1 = std::max(0.0f, dotProduct(ws, N));
        float dot2 = std::max(0.0f, dotProduct(-ws, NN));
        //distance betwin x and hit point 
		float distance2 = (x - hitPoint).norm();
		distance2 = distance2 * distance2;
        L_dir = emit * eval * dot1 * dot2 / distance2 / pdf_light;
    }

    // === Indirect Lighting (Russian Roulette) ===
    if (get_random_float() < RussianRoulette) {
        // Vector3f wi = hit.m->sample(wo, N);
        Vector3f wi = hit.m->sample(wo, N);
        Ray newRay(hitPoint, wi);
        Intersection newHit = intersect(newRay);

        if (newHit.happened && !newHit.m->hasEmission()) {
            Vector3f eval = hit.m->eval(wo, wi, N) ;
            float pdf_val = hit.m->pdf(wo, wi, N);
            float cos_theta = std::max(0.0f, dotProduct(wi, N));
            L_indir = castRay(newRay) * eval * cos_theta / pdf_val / RussianRoulette;
        }
    }

    return L_dir + L_indir;
}