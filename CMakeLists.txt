cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

project(fluids)

add_library(main main.cpp)

add_library(matrix matrix.cpp)

find_package(glfw3 3.3 REQUIRED)
target_link_libraries(main glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(main OpenGL::GL)

find_package(fmt)
target_link_libraries(main fmt::fmt)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -I/usr/local/lib -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL -lglfw")

add_executable(fluids main)
target_link_libraries(fluids matrix)
