
void instantiate_sphere_cylinder_scene(Scene& scene) {
  // Two cylinders side by side (replacing cubes from the cube scene)
  auto cyl1 = std::make_unique<Cylinder>(Vector3f(-3.0f, -1.0f, -12.0f), 1.2f, 4.0f);
  cyl1->materialType = DIFFUSE_AND_GLOSSY;
  cyl1->diffuseColor = Vector3f(0.9f, 0.1f, 0.1f);
  cyl1->Kd = 0.8f;
  cyl1->Ks = 0.5f;
  cyl1->specularExponent = 32.0f;
  scene.Add(std::move(cyl1));

  auto cyl2 = std::make_unique<Cylinder>(Vector3f(3.0f, -1.0f, -12.0f), 1.2f, 4.0f);
  cyl2->materialType = DIFFUSE_AND_GLOSSY;
  cyl2->diffuseColor = Vector3f(0.1f, 0.1f, 0.9f);
  cyl2->Kd = 0.8f;
  cyl2->Ks = 0.5f;
  cyl2->specularExponent = 32.0f;
  scene.Add(std::move(cyl2));

  // Two spheres in the foreground — one reflective/refractive each
  auto sphere1 = std::make_unique<Sphere>(Vector3f(2.0f, -1.0f, -6.0f), 1.0f);
  sphere1->materialType = REFLECTION_AND_REFRACTION;
  scene.Add(std::move(sphere1));

  auto sphere2 = std::make_unique<Sphere>(Vector3f(-2.0f, -1.0f, -6.0f), 1.0f);
  sphere2->materialType = REFLECTION_AND_REFRACTION;
  scene.Add(std::move(sphere2));
}

void instantiate_main_scene(Scene& scene) {
  float x_displacement = -1 - 3;
  auto sph1 = std::make_unique<Sphere>(Vector3f(x_displacement, 0, -12), 2);
  sph1->materialType = DIFFUSE_AND_GLOSSY;
  sph1->diffuseColor = Vector3f(0.6, 0.7, 0.8);
  scene.Add(std::move(sph1));
  x_displacement = 1 - 3;
  auto sph2 = std::make_unique<Sphere>(Vector3f(x_displacement, -0.5, -8), 1.5);
  sph2->ior = 1.5;
  sph2->materialType = REFLECTION_AND_REFRACTION;
  scene.Add(std::move(sph2));
  x_displacement = 4 + 2;
  auto sphere3 = std::make_unique<Sphere>(Vector3f(x_displacement, -1, -12), 2.0f);
  sphere3->materialType = DIFFUSE_AND_GLOSSY;
  sphere3->diffuseColor = Vector3f(1.0f, 0.1f, 1.0f);   // vivid blue
  sphere3->Kd = 1.0f;           // strong diffuse component
  sphere3->Ks = 1.0f;           // mild specular highlight
  sphere3->specularExponent = 50.0f; // moderately sharp highlight
  scene.Add(std::move(sphere3));
  x_displacement = .8 + 2;
  auto sphere4 = std::make_unique<Sphere>(Vector3f(x_displacement, -1, -8), 1.5);
  sphere4->materialType = REFLECTION_AND_REFRACTION;
  scene.Add(std::move(sphere4));
}



void instantiate_sphere_material_scene(Scene& scene) {
  float x_displacement = -3;
  auto sphere3 = std::make_unique<Sphere>(Vector3f(x_displacement, -1, -12), 1.5);
  sphere3->materialType = DIFFUSE_AND_GLOSSY;
  sphere3->diffuseColor = Vector3f(0.1, 0.9, 0.1);
  sphere3->Kd = 1.0f;           // strong diffuse component
  sphere3->Ks = 1.0f;           // mild specular highlight
  scene.Add(std::move(sphere3));
  x_displacement = 3;
  auto sphere4 = std::make_unique<Sphere>(Vector3f(x_displacement, -1, -12), 1.5);
  sphere4->materialType = DIFFUSE_AND_GLOSSY;
  sphere4->diffuseColor = Vector3f(0.1, 0.1, 0.9);
  sphere4->Kd = 1.0f;           // strong diffuse component
  sphere4->Ks = 1.0f;           // mild specular highlight
  scene.Add(std::move(sphere4));

  auto sphere1 = std::make_unique<Sphere>(Vector3f(2, -1, -6), 1);
  sphere1->materialType = DIFFUSE_AND_GLOSSY;
  sphere1->diffuseColor = Vector3f(0.1, 0.5, 0.5);
  sphere1->Kd = 1.0f;           // strong diffuse component
  sphere1->Ks = 1.0f;    
  scene.Add(std::move(sphere1));

  auto sphere2 = std::make_unique<Sphere>(Vector3f(-2, -1, -6), 1);
  sphere2->materialType = DIFFUSE_AND_GLOSSY;
  sphere2->diffuseColor = Vector3f(0.9, 0.1, 0.9);
  sphere2->Kd = 1.0f;           // strong diffuse component
  sphere2->Ks = 1.0f;  
  // sphere2->ior = 1.5;
  scene.Add(std::move(sphere2));
  
}

void instantiate_sphere_cube_scene(Scene& scene) {
  float x_displacement = -3;
  auto cube1 = std::make_unique<Cube>(Vector3f(x_displacement, -2, -12), Vector3f (-1,0,-10));
  cube1->materialType = DIFFUSE_AND_GLOSSY;
  cube1->diffuseColor = Vector3f(0.9, 0.1, 0.1);
  cube1->Kd = 1.0f;           // strong diffuse component
  cube1->Ks = 1.0f;           // mild specular highlight
  scene.Add(std::move(cube1));
  x_displacement = 1;
  auto cube2 = std::make_unique<Cube>(Vector3f(x_displacement, -2, -12), Vector3f (3,0,-10));
  cube2->materialType = DIFFUSE_AND_GLOSSY;
  cube2->diffuseColor = Vector3f(0.1, 0.1, 0.9);
  cube2->Kd = 1.0f;           // strong diffuse component
  cube2->Ks = 1.0f;           // mild specular highlight
  scene.Add(std::move(cube2));

  auto sphere1 = std::make_unique<Sphere>(Vector3f(2, -1, -6), 1);
  sphere1->materialType = REFLECTION_AND_REFRACTION; 
  scene.Add(std::move(sphere1));

  auto sphere2 = std::make_unique<Sphere>(Vector3f(-2, -1, -6), 1);
  sphere2->materialType = REFLECTION_AND_REFRACTION;  
  // sphere2->ior = 1.5;
  scene.Add(std::move(sphere2));
  
}