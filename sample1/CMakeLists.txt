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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
message("Binary dir: ${CMAKE_BINARY_DIR}")

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
add_custom_target(hello_images)
add_custom_command(TARGET hello_images PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E
        copy ${CMAKE_SOURCE_DIR}/res/hello.bmp ${CMAKE_BINARY_DIR}/bin/)

set(HELLO_SOURCES
    source/sdl_hello/TextureWrapper.h
    source/sdl_hello/main.cpp 
)
add_executable(sdl_hello ${HELLO_SOURCES})
add_dependencies(sdl_hello hello_images)
target_link_libraries(sdl_hello gfx ${SDL2_LIBRARIES})

##########

set(SPIRVTOOLS ${CMAKE_CURRENT_SOURCE_DIR}/tools/linux/bin)
set(GLSLCOMPILER ${SPIRVTOOLS}/glslc)

add_custom_target(shaders)
file(GLOB ShaderFiles 
    ${CMAKE_SOURCE_DIR}/res/shaders/*.frag
    ${CMAKE_SOURCE_DIR}/res/shaders/*.vert)

# add_custom_command(TARGET shaders PRE_BUILD
#                  COMMAND ${CMAKE_COMMAND} -E copy ${shaderFile} ${CMAKE_BINARY_DIR}/bin/shaders/)
add_custom_command(TARGET shaders PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/shaders)
foreach(shaderFile ${ShaderFiles})
    get_filename_component(FILENAME ${shaderFile} NAME)
    message("Adding shader: ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv")
    add_custom_command(TARGET shaders PRE_BUILD
        # OUTPUT  ${FILENAME}.spv          # Treated as relative to CMAKE_CURRENT_BINARY_DIR
        COMMAND ${GLSLCOMPILER} ${shaderFile} -o ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv
        DEPENDS ${shaderFile} # Treated as relative to CMAKE_CURRENT_SOURCE_DIR
    )
endforeach()

file(GLOB_RECURSE VK_HELLO_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "source/vk_hello/*.c*")
add_executable(vk_hello ${VK_HELLO_SOURCES})
add_dependencies(vk_hello shaders)
target_link_libraries(vk_hello gfx ${SDL2_LIBRARIES} vulkan)
# how to include libshaderc static from shaderc external 
#target_link_libraries(vk_hello gfx ${SDL2_LIBRARIES} vulkan shaderc)
