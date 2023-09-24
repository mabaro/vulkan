# tuto: https://medium.com/@onur.dundar1/cmake-tutorial-585dd180109b

cmake_minimum_required(VERSION 3.9.1)
project(CMakeHello)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
add_compile_options("$<$<STREQUAL:$<TARGET_PROPERTY:LINKER_LANGUAGE>,CXX>:-std=c++11>")

if(MSVC)
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>")
else()
    add_compile_options("-Wall" "-Wextra" "$<$<CONFIG:RELEASE>:-O3>")
    # add_compile_options("-Wall" "-Wextra")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    message("Debug Build")
elseif(${CMAKE_BUILD_TYPE} MATCHES Release)
    message("Release Build")
endif()

# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()

# CMAKE_SYSTEM
# CMAKE_SYSTEM_NAME
# CMAKE_SYSTEM_VERSION
# CMAKE_SYSTEM_PROCESSOR
# CMAKE_HOST_SYSTEM_NAME
if(UNIX)
    message("This is a ${CMAKE_SYSTEM_NAME} system")
elseif(WIN32)
    message("This is a Windows System")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    message("This is a ${CMAKE_SYSTEM_NAME} system")
elseif(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    message("This is a Windows System")
endif()

## shared libraries
# LIBRARY_OUTPUT_PATH
# CMAKE_LIBRARY_OUTPUT_DIRECTORY
## static libraries
# ARCHIVE_OUTPUT_PATH
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY

add_subdirectory(lib/gfx)

# ## shaderc libshaderc
# # include out of tree library needs binary dir as second param
# set(SHADERC_SKIP_EXAMPLES ON)
# set(SHADERC_SKIP_TESTS ON)
# set(SHADERC_SKIP_INSTALL ON)
# set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
# add_subdirectory(../shaderc-main glslc)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

##########
## IMGUI
SET(THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
add_library(imgui
    "${THIRDPARTY}/imgui/imgui.h"
    "${THIRDPARTY}/imgui/imgui.cpp"
    "${THIRDPARTY}/imgui/imgui_demo.cpp"
    "${THIRDPARTY}/imgui/imgui_draw.cpp"
    "${THIRDPARTY}/imgui/imgui_widgets.cpp"
    "${THIRDPARTY}/imgui/backends/imgui_impl_vulkan.cpp"
    "${THIRDPARTY}/imgui/backends/imgui_impl_sdl2.cpp"
)
target_include_directories(imgui PUBLIC "${THIRDPARTY}/imgui/")

############################################################

add_subdirectory(sdl_hello)

add_subdirectory(vk_hello)
