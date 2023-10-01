cmake_minimum_required(VERSION 3.9.1)
project(core)
message("${PROJECT_NAME} library")

set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "source/*.h*")
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "source/*.c*")

add_library(core SHARED ${SOURCES} ${HEADERS})
target_include_directories(core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>  # <prefix>/include/mylib
)
set_target_properties(core PROPERTIES
    LINKER_LANGUAGE CXX
    COMPILE_FLAGS "-std=c++11"
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libcore.so"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)


#############################################

if (ENABLE_TESTS)
  add_subdirectory(test)
endif()
