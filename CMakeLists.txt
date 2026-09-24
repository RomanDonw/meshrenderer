cmake_minimum_required(VERSION 3.23)
project(meshrenderer LANGUAGES C)

add_library(glad OBJECT "external/glad/glad.c")
target_include_directories(glad PUBLIC "external/glad/")

add_library(stb_image OBJECT "external/stb_image/stb_image.c")
target_include_directories(stb_image PUBLIC "external/stb_image/")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
unset(SOURCES)

find_package(glfw3 3.3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad stb_image)
