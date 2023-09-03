cmake_minimum_required(VERSION 3.9.1)

set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "source/*.c*")
set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11")
add_library(gfx SHARED ${SOURCES})
set_target_properties(gfx PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libgfx.so"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)
