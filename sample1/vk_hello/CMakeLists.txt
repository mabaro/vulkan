set(APP_NAME vk_hello)
set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}/${APP_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
message("Binary dir for(${APP_NAME}): ${CMAKE_BINARY_DIR}")

set(SPIRVTOOLS ${CMAKE_SOURCE_DIR}/tools/linux/bin)
set(GLSLCOMPILER ${SPIRVTOOLS}/glslc)

add_custom_target(shaders)
file(GLOB ShaderFiles 
    ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/*.vert)

set(SHADERS_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH}/shaders)
message("Shaders will be written into: ${SHADERS_OUTPUT_DIR}")
add_custom_command(TARGET shaders PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_OUTPUT_DIR})
foreach(shaderFile ${ShaderFiles})
    get_filename_component(FILENAME ${shaderFile} NAME)
    message("Adding shader: ${FILENAME}.spv")
    add_custom_command(TARGET shaders PRE_BUILD
        COMMAND ${GLSLCOMPILER} ${shaderFile} -o ${SHADERS_OUTPUT_DIR}/${FILENAME}.spv
        DEPENDS ${shaderFile} # Treated as relative to CMAKE_CURRENT_SOURCE_DIR
    )
endforeach()

file(GLOB_RECURSE VK_HELLO_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "source/*.c*")

add_executable(vk_hello ${VK_HELLO_SOURCES})
add_dependencies(vk_hello shaders)
target_link_libraries(vk_hello gfx SDL2::SDL2 imgui vulkan math)
# how to include libshaderc static from shaderc external 
#target_link_libraries(vk_hello gfx SDL2 vulkan shaderc)
# set_property(TARGET vk_hello PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# set_property(TARGET sdl_hello PROPERTY VS_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

message(vk_hello.)