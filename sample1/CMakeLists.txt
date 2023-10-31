# tuto: https://medium.com/@onur.dundar1/cmake-tutorial-585dd180109b

cmake_minimum_required(VERSION 3.9.1)
project(VulkanSamples)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#################################################3
# dependencies

###
find_package(abc REQUIRED)

###
set(SDL_SHARED OFF)
find_package(SDL2 REQUIRED PATH thirdparty/SDL2)

###
# set(ENV{VULKAN_SDK_PATH} "D:/0-repositories/thirdparties/vulkan_sdk")
if (DEFINED SDK_VULKAN_PATH)
    set(Vulkan_INCLUDE_DIRS ${SDK_VULKAN_PATH}/include)
    set(Vulkan_LIBRARIES ${SDK_VULKAN_PATH}/lib)
    set(Vulkan_FOUND "true")
else()
    find_package(Vulkan REQUIRED)
endif()
if (Vulkan_FOUND)
    message("Vulkan FOUND @ ${Vulkan_INCLUDE_DIRS}")
else()
    message("Vulkan NOT FOUND - VULKAN_SDK_PATH not found")
endif()

###
option( ENABLE_TESTS "Enable tests" OFF )
if (ENABLE_TESTS)
    message("------- TESTS ENABLED -------")
    include(CTest)
    # find_package(doctest QUIET)
    find_package(GTest QUIET)
else()
    message("------- TESTS DISABLED -------")
endif()

################################################
# compilation configuration

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(MSVC)
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>")
    add_compile_options("/EHsc")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:console")
    add_definitions(-DUNICODE -D_UNICODE)
else()
    add_compile_options("-Wall" "-Wextra" "$<$<CONFIG:RELEASE>:-O3>")
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

#############################################################
# build targets

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(lib)

# ## shaderc libshaderc
# # include out of tree library needs binary dir as second param
# set(SHADERC_SKIP_EXAMPLES ON)
# set(SHADERC_SKIP_TESTS ON)
# set(SHADERC_SKIP_INSTALL ON)
# set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
# add_subdirectory(../shaderc-main glslc)

##########
## IMGUI
SET(THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
add_library(imgui
    "${THIRDPARTY}/imgui/imgui.h"
    "${THIRDPARTY}/imgui/imgui.cpp"
    "${THIRDPARTY}/imgui/imgui_demo.cpp"
    "${THIRDPARTY}/imgui/imgui_draw.cpp"
    "${THIRDPARTY}/imgui/imgui_widgets.cpp"
    "${THIRDPARTY}/imgui/imgui_tables.cpp"
    "${THIRDPARTY}/imgui/backends/imgui_impl_vulkan.cpp"
    "${THIRDPARTY}/imgui/backends/imgui_impl_sdl2.cpp"
)
target_include_directories(imgui PUBLIC "${THIRDPARTY}/imgui/")
target_link_libraries(imgui PUBLIC SDL2 Vulkan::Vulkan)

############################################################

add_subdirectory(sdl_hello)

add_subdirectory(vk_hello)

############################################################

if (ENABLE_TESTS)
    enable_testing()
endif()
