#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include "scene.hpp"
#include "surface_scene.hpp"
#include "target_scene.hpp"
#include "view.hpp"

namespace {

cv::Mat render_scene(const Scene& scene) {
    cv::Mat image(kHeight, kWidth, CV_8UC3, cv::Scalar(0, 0, 0));
    scene.render(image);
    return image;
}

void print_usage(const char* exe) {
    std::cerr
        << "Usage:\n"
        << "  " << exe << " <linear|bezier> <num_points> <rotX> <rotY> <rotZ> [output.png]\n"
        << "  " << exe << " target [<rotX> <rotY> <rotZ> [output.png]]\n"
        << "Angles are in degrees. With an output file the image is saved and\n"
        << "no window is opened.\n";
}

// Window loop: TAB switches between the scenes, Esc quits.
int run_interactive(std::vector<Scene*> scenes, int current) {
    scenes[current]->print_controls();
    int key = 0;
    while (key != 27) {
        cv::imshow("Bezier Surface 3D", render_scene(*scenes[current]));
        key = cv::waitKey(0);
        if (key == '\t') {
            current = (current + 1) % scenes.size();
            scenes[current]->print_controls();
        } else if (key > 0) {
            scenes[current]->on_key(key);
        }
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    // Render everything to ./output:
    //   ./BezierCurve --all
    if (args.size() == 1 && args[0] == "--all") {
        namespace fs = std::filesystem;
        fs::path out_dir = "output";
        fs::create_directories(out_dir);
        struct { float rx, ry, rz; std::string label; } views[] = {
            { 90.f,  0.f, 0.f, "x90" },
            {  0.f, 90.f, 0.f, "y90" },
            {  0.f,  0.f, 0.f, "z0"  },
        };
        for (const auto& v : views)
            for (bool bezier : {true, false}) {
                fs::path file = out_dir / ((bezier ? "bezier_" : "linear_") + v.label + ".png");
                cv::imwrite(file.string(), render_scene(SurfaceScene(bezier, 10, v.rx, v.ry, v.rz)));
                std::cout << "saved " << file.string() << "\n";
            }
        for (const auto& v : {views[0], views[2]}) {
            fs::path file = out_dir / ("target_" + v.label + ".png");
            cv::imwrite(file.string(), render_scene(TargetScene(v.rx, v.ry, v.rz)));
            std::cout << "saved " << file.string() << "\n";
        }
        return 0;
    }

    // Target scene:
    //   ./BezierCurve target                                   (window)
    //   ./BezierCurve target <rotX> <rotY> <rotZ> [output.png]
    if (!args.empty() && args[0] == "target") {
        if (args.size() != 1 && args.size() != 4 && args.size() != 5) {
            print_usage(argv[0]);
            return -1;
        }
        float rx = 0, ry = 0, rz = 0;
        if (args.size() >= 4) {
            rx = std::atof(args[1].c_str());
            ry = std::atof(args[2].c_str());
            rz = std::atof(args[3].c_str());
        }
        TargetScene target(rx, ry, rz);
        if (args.size() == 5) {
            cv::imwrite(args[4], render_scene(target));
            std::cout << "saved " << args[4] << "\n";
            return 0;
        }
        SurfaceScene surface(true, 10, 0, 0, 0);
        return run_interactive({&surface, &target}, 1);
    }

    // Surface scene:
    //   ./BezierCurve <linear|bezier> <num_points> <rotX> <rotY> <rotZ> [output.png]
    if (args.size() != 5 && args.size() != 6) {
        print_usage(argv[0]);
        return -1;
    }
    bool bezier;
    if (args[0] == "linear") {
        bezier = false;
    } else if (args[0] == "bezier") {
        bezier = true;
    } else {
        std::cerr << "Invalid curve mode: " << args[0] << "\n";
        print_usage(argv[0]);
        return -1;
    }
    int num_points = std::max(2, std::atoi(args[1].c_str()));
    SurfaceScene surface(bezier, num_points, std::atof(args[2].c_str()),
                         std::atof(args[3].c_str()), std::atof(args[4].c_str()));
    if (args.size() == 6) {
        cv::imwrite(args[5], render_scene(surface));
        std::cout << "saved " << args[5] << "\n";
        return 0;
    }
    TargetScene target;
    return run_interactive({&surface, &target}, 0);
}
