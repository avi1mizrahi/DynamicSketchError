cmake_minimum_required(VERSION 3.15)
project(sketch_ring)

add_subdirectory(massdalsketches)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Wall -Werror)

# see https://cmake.org/cmake/help/latest/module/FindBoost.html
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_executable(dynamic_sketch
               playground.cxx
               CountMinSketch.h
               Sketch.h
               )
target_link_libraries(dynamic_sketch massdal)
