cmake_minimum_required(VERSION 3.10)
project(NVWorldEdit CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(NVWorldEdit source/main.cpp source/main.h source/GameData.cpp source/GameData.h source/GameMasterData.cpp source/GameMasterData.h)
target_link_libraries(NVWorldEdit "-lwinmm -lopengl32 -lgdi32 -lkernel32 -lComctl32")
