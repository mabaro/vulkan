cmake_minimum_required(VERSION 3.9.1)

set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "source/*.h*")
file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "source/*.c*")

add_library(gfx STATIC ${SOURCES} ${HEADERS})
target_include_directories(gfx PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>  # <prefix>/include/mylib
)
target_link_libraries(gfx PUBLIC core math)
set_target_properties(gfx PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 11
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libgfx.so"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

