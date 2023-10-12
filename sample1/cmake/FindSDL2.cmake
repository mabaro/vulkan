if(TARGET SDL2::SDL2)
    return()
endif()

if (SDL2_FOUND)
    return()
endif()

include(FetchContent)
message(STATUS "Third-party: creating target 'SDL2::SDL2'")
# if (FMT_USE_THIRDPARTY)
#     FetchContent_Declare(
    #         fmt
    #         SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/fmt/"
    #     )
    # else()
    message("Fetching SDL2 library")
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG SDL2
    )
    FetchContent_MakeAvailable(SDL2)
# endif()
