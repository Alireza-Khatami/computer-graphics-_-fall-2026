cmake_minimum_required(VERSION 3.10)
project(Rasterizer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
# On Ubuntu, Eigen ships a CMake config that defines the imported target Eigen3::Eigen
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

add_executable(Rasterizer
    main.cpp
    rasterizer.hpp rasterizer.cpp
    Triangle.hpp  Triangle.cpp
)

# Link libraries (OpenCV provides include dirs via the target, Eigen is header-only)
target_link_libraries(Rasterizer PRIVATE ${OpenCV_LIBS} Eigen3::Eigen)