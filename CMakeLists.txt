cmake_minimum_required(VERSION 3.10)
project(lvgl_watch)

set(CMAKE_C_STANDARD 99)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
)

# LVGL source files
file(GLOB_RECURSE LVGL_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src/*.c
)

# Main application
set(SOURCES
    ${LVGL_SOURCES}
    src/main.c
)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} m pthread)