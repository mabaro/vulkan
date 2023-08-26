cmake_minimum_required(VERSION 3.9.1)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp PROPERTIES COMPILE_FLAGS "-std=c++11")
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
add_library(gfx SHARED 
    include/device.h
    source/device.cpp
)
