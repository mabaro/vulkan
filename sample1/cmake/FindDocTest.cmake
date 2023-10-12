if(TARGET doctest::doctest)
    return()
endif()

if (doctest_FOUND)
    return()
endif()

message(STATUS "Third-party: creating target 'doctest::doctest'")
# if (FMT_USE_THIRDPARTY)
#     FetchContent_Declare(
#         fmt
#         SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/fmt/"
#     )
# else()
    include(FetchContent)
    message("Fetching doctest on sample1")
    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.h
        GIT_TAG HEAD
    )
    FetchContent_MakeAvailable(doctest)
    set(doctest_DIR ${doctest_BINARY_DIR})
    message("doctest dir: " ${doctest_BINARY_DIR})
    FetchContent_MakeAvailable(doctest)
# endif()
