#include <chrono>

#include "Renderer.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv) {
    int spp;
    int width, height;
    std::string outputfile;
    if (argc==4){
        spp = atoi(argv[1]);
        width= atoi(argv[2]);
        height= atoi(argv[3]);
        outputfile= "output_"+std::to_string(spp)+"spp_"+std::to_string(width)+"x"+std::to_string(height)+".png";
    } else {
        spp = 16;
        width= 512;
        height= 512;
        outputfile= "output.png";
    }

  // Change the definition here to change resolution
  Scene normal_scene(width, height);
  Material* red = new Material(DIFFUSE, Vector3f(0.0f));
  red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
  Material* green = new Material(DIFFUSE, Vector3f(0.0f));
  green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
  Material* white = new Material(DIFFUSE, Vector3f(0.0f));
  white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
  Material* light = new Material(
      DIFFUSE, (8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) +
                15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) +
                18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)));
  light->Kd = Vector3f(0.65f);

  // TODO: copy and paste folder path there, DO NOT use relative path
  MeshTriangle floor("../models/cornellbox/floor.obj", white);
  MeshTriangle shortbox("../models/cornellbox/shortbox.obj", white);
  MeshTriangle tallbox("../models/cornellbox/tallbox.obj", white);
  MeshTriangle left("../models/cornellbox/left.obj", red);
  MeshTriangle right("../models/cornellbox/right.obj", green);
  MeshTriangle light_("../models/cornellbox/light.obj", light);

  normal_scene.Add(&floor);
  normal_scene.Add(&shortbox);
  normal_scene.Add(&tallbox);
  normal_scene.Add(&left);
  normal_scene.Add(&right);
  normal_scene.Add(&light_);

  normal_scene.buildBVH();
  Renderer r;

  // auto start = std::chrono::system_clock::now();
  // r.Render(normal_scene, spp);
  // auto stop = std::chrono::system_clock::now();

  auto start = std::chrono::system_clock::now();
  r.Render(normal_scene, spp);
  auto stop = std::chrono::system_clock::now();
  

  std::cout << "Render complete: \n";
  std::cout
      << "Time taken: "
      << std::chrono::duration_cast<std::chrono::hours>(stop - start).count()
      << " hours\n";
  std::cout
      << "          : "
      << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count()
      << " minutes\n";
  std::cout
      << "          : "
      << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
      << " seconds\n";

  return 0;
}