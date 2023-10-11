set(TARGET_NAME ${PROJECT_NAME})
message("${TARGET_NAME} - TESTS")

include(GoogleTest)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c*")
set(TEST_NAME ${TARGET_NAME}_test)

add_executable(${TEST_NAME} ${SOURCES})
target_link_libraries(${TEST_NAME} PRIVATE ${TARGET_NAME} GTest::gtest_main abc)
gtest_discover_tests(${TEST_NAME})

add_subdirectory(performance)