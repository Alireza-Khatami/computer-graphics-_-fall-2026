cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

# This is a per-pixel CPU rasterizer; an unoptimized Debug build runs at a
# few frames per second on a 700x700 canvas. Default to Release so the
# interactive window is actually usable unless the user asks for Debug.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(OpenCV REQUIRED)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

set(CMAKE_CXX_STANDARD 17)

include_directories("/usr/local/include/eigen3")

add_executable(Rasterizer
  main.cpp
  rasterizer.hpp rasterizer.cpp
  Triangle.hpp Triangle.cpp
  scene.hpp
  transforms.hpp
  triangle_scene.hpp triangle_scene.cpp
  cube_scene.hpp cube_scene.cpp)
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES} Eigen3::Eigen)
