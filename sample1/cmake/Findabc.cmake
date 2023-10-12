if(TARGET abc::abc)
    return()
endif()

if (ABC_FOUND)
    message(STATUS "Third-party: ABC FOUND")
    return()
endif()

message(STATUS "Third-party: creating target 'abc::abc'")
include(FetchContent)
message("Fetching ABC library")
# if (FMT_USE_THIRDPARTY)
#     FetchContent_Declare(
#         fmt
#         SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/fmt/"
#     )
# else()
FetchContent_Declare(
    abc
    GIT_REPOSITORY https://github.com/mabaro/abc.git
    GIT_TAG main#0.5-alpha
)
# endif()
FetchContent_MakeAvailable(abc)
