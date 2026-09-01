#include <chrono>

#include "Renderer.hpp"
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Vector.hpp"
#include "global.hpp"

int main(int argc, char** argv) {
    int spp, width, height;
    if (argc == 4) {
        spp    = atoi(argv[1]);
        width  = atoi(argv[2]);
        height = atoi(argv[3]);
    } else {
        spp    = 16;
        width  = 128;
        height = 128;
    }

    Renderer r;

    // ── Scene 1: Normal Cornell Box (all diffuse) ────────────────────────
    {
        Scene normal_scene(width, height);
        Material* red   = new Material(DIFFUSE, Vector3f(0.0f));
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
        MeshTriangle floor     ("../models/cornellbox/floor.obj",    white);
        MeshTriangle shortbox  ("../models/cornellbox/shortbox.obj", white);
        MeshTriangle tallbox   ("../models/cornellbox/tallbox.obj",  white);
        MeshTriangle left_wall ("../models/cornellbox/left.obj",     red);
        MeshTriangle right_wall("../models/cornellbox/right.obj",    green);
        MeshTriangle light_    ("../models/cornellbox/light.obj",    light);

        normal_scene.Add(&floor);
        normal_scene.Add(&shortbox);
        normal_scene.Add(&tallbox);
        normal_scene.Add(&left_wall);
        normal_scene.Add(&right_wall);
        normal_scene.Add(&light_);
        normal_scene.buildBVH();

        std::cout << "\n=== Rendering Scene 1: Diffuse Cornell Box ===\n";
        auto start = std::chrono::system_clock::now();
        r.Render(normal_scene, spp, "binary_diffuse.ppm");
        auto stop  = std::chrono::system_clock::now();
        std::cout << "Time taken: "
                  << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
                  << " seconds\n";
    }

    // ── Scene 2: Glass tall box Cornell Box ──────────────────────────────
    // {
    //     Scene glass_scene(width, height);
    //     Material* red   = new Material(DIFFUSE, Vector3f(0.0f));
    //     red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    //     Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    //     green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    //     // Glass material for the tall box — ior 1.5 (crown glass)
    //     Material* glass_box = new Material(GLASS, Vector3f(0.0f));
    //     glass_box->ior = 1.5f;
    //     Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    //     white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    //     Material* light = new Material(
    //         DIFFUSE, (8.0f * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f) +
    //                   15.6f * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) +
    //                   18.4f * Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f)));
    //     light->Kd = Vector3f(0.65f);
    //
    //     // TODO: copy and paste folder path there, DO NOT use relative path
    //     MeshTriangle floor     ("../models/cornellbox/floor.obj",    white);
    //     MeshTriangle shortbox  ("../models/cornellbox/shortbox.obj", white);
    //     MeshTriangle tallbox   ("../models/cornellbox/tallbox.obj",  glass_box);
    //     MeshTriangle left_wall ("../models/cornellbox/left.obj",     red);
    //     MeshTriangle right_wall("../models/cornellbox/right.obj",    green);
    //     MeshTriangle light_    ("../models/cornellbox/light.obj",    light);
    //
    //     glass_scene.Add(&floor);
    //     glass_scene.Add(&shortbox);
    //     glass_scene.Add(&tallbox);
    //     glass_scene.Add(&left_wall);
    //     glass_scene.Add(&right_wall);
    //     glass_scene.Add(&light_);
    //     glass_scene.buildBVH();
    //
    //     std::cout << "\n=== Rendering Scene 2: Glass Tall Box ===\n";
    //     auto start = std::chrono::system_clock::now();
    //     r.Render(glass_scene, spp, "binary_glass.ppm");
    //     auto stop  = std::chrono::system_clock::now();
    //     std::cout << "Time taken: "
    //               << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count()
    //               << " seconds\n";
    // }

    return 0;
}
