cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

find_package(OpenCV REQUIRED)


set(CMAKE_CXX_STANDARD 17)

# Default to an optimized build: unoptimized Eigen makes the software
# rasterizer ~35x slower, which makes the interactive camera unusable.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()


# include_directories("usr/include/eigen3")


add_executable(Rasterizer main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)
target_link_libraries(Rasterizer ${OpenCV_LIBRARIES} )
# target_link_directories(Rasterizer PRIVATE /usr/include/eigen3/Eigen)