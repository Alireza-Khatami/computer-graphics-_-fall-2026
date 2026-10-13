cmake_minimum_required(VERSION 3.10)
project(BezierCurve)

find_package(OpenCV REQUIRED)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

set(CMAKE_CXX_STANDARD 17)

# Default to an optimized build; unoptimized Eigen is much slower.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

include_directories("/usr/local/include/eigen3")

add_executable(BezierCurve main.cpp
               transforms.hpp transforms.cpp bezier.hpp bezier.cpp
               view.hpp scene.hpp
               surface_scene.hpp surface_scene.cpp
               target_scene.hpp target_scene.cpp)

target_link_libraries(BezierCurve ${OpenCV_LIBRARIES} Eigen3::Eigen)
