cmake_minimum_required(VERSION 3.10)
project(NVWorldEdit CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(NVWorldEdit source/main.cxx source/main.hxx source/GameData.cxx source/GameData.hxx source/GameMasterData.cxx source/GameMasterData.hxx)
target_link_libraries(NVWorldEdit "-lwinmm -lopengl32 -lgdi32 -lkernel32 -lComctl32")
