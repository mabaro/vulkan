if(TARGET GTest::GTest)
    return()
endif()
if (GTEST_FOUND)
    return()
endif()

message(STATUS "Third-party: creating target 'GTest::GTest'")

# if (FMT_USE_THIRDPARTY)
#     FetchContent_Declare(
#         fmt
#         SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/fmt/"
#     )
# else()
    include(FetchContent)
    FetchContent_Declare(
        GTest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(GTest)
# endif()
