cmake_minimum_required(VERSION 3.10)
project(NVWorldEdit CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(NVWorldEdit main.cpp)
target_link_libraries(NVWorldEdit "-lwinmm -lopengl32 -lglu32 -lgdi32 -lkernel32")
