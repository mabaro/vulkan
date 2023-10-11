if(TARGET SDL2::SDL2)
    return()
endif()

message(STATUS "Third-party: creating target 'SDL2::SDL2'")

include(FetchContent)

message("Fetching SDL2 library")

# if (FMT_USE_THIRDPARTY)
#     FetchContent_Declare(
#         fmt
#         SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/fmt/"
#     )
# else()
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main#0.5-alpha
    )
# endif()

FetchContent_MakeAvailable(SDL2)