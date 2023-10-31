if(TARGET SDL2::SDL2)
    return()
endif()

if (SDL2_FOUND)
    return()
endif()

message(STATUS "Third-party: creating target 'SDL2::SDL2'")

# if (DEFINED SDL2_PATH)
#     message(STATUS "Third-party: 'SDL2::SDL2' FOUND")
#     set(SDL2_INCLUDE_DIRS ${SDL2_PATH}/include)
#     set(SDL2_LIBRARIES ${SDL2_PATH}/lib)
#     set(SDL2_FOUND "true")
# else()
include(FetchContent)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG SDL2
    GIT_PROGRESS TRUE
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2
    SUBBUILD_DIR      thirdparty/SDL2/subbuild
    BINARY_DIR        thirdparty/SDL2/build
)
FetchContent_MakeAvailable(SDL2)

if (TARGET SDL2::SDL2)
    message("SDL2 FOUND @ ${SDL2_SOURCE_DIR}")
else()
    message("SDL2 NOT FOUND")
endif()
