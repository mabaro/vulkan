set(APP_NAME sdl_hello)
set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}/${APP_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)
message("Binary dir for(${APP_NAME}): ${CMAKE_BINARY_DIR}")

add_custom_target(hello_images)
add_custom_command(TARGET hello_images PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/res/hello.bmp ${EXECUTABLE_OUTPUT_PATH})

set(HELLO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/TextureWrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp 
)
add_executable(sdl_hello ${HELLO_SOURCES})
add_dependencies(sdl_hello hello_images)
target_link_libraries(sdl_hello gfx ${SDL2_LIBRARIES})
# set_property(TARGET sdl_hello PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
# set_property(TARGET sdl_hello PROPERTY VS_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
