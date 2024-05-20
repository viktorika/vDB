cmake_minimum_required (VERSION 3.27.2)

project(vdb VERSION 1.0.0)

file(GLOB_RECURSE SOURCES "src/*.cc")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_compile_options(-O2)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vdb STATIC ${SOURCES})
target_include_directories(vdb PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(vdb PRIVATE src src/memtable src/util)