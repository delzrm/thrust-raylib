cmake_minimum_required(VERSION 3.15)
project(thrust C)

set(CMAKE_C_STANDARD 11)

# Download raylib automatically
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(thrust src/thrust.c src/Draw.c src/Levels.c src/Collision.c)
target_link_libraries(thrust raylib)

if(MSVC)
    target_compile_options(thrust PRIVATE /W3)
else()
    target_compile_options(thrust PRIVATE -Wall -Wextra)
endif()

# Copy executable to project root for easy running
set_target_properties(thrust PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
