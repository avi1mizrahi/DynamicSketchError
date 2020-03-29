cmake_minimum_required(VERSION 3.0)
project(sketch_ring)


add_subdirectory(massdalsketches)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-Wall -Werror)

add_executable(dynamic_sketch playground.cxx Cm.h)
target_link_libraries(dynamic_sketch massdal)
