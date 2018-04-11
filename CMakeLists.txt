cmake_minimum_required(VERSION 3.10)
project(NVWorldEdit C)

set(CMAKE_C_STANDARD 99)

add_executable(NVWorldEdit main.c)
target_link_libraries(NVWorldEdit "-lwinmm -lopengl32 -lglu32 -lgdi32 -lkernel32")
