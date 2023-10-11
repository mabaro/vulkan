set(TARGET_NAME ${PROJECT_NAME})
project(${TARGET_NAME}_performance)
message("${TARGET_NAME} - PERFORMANCE")

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c*")

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} math abc)
