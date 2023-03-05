cmake_minimum_required(VERSION 3.23)

project(neon)

set(CMAKE_CXX_STANDARD 17)

SET(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "")
SET(SDL_SHARED OFF CACHE BOOL "")
SET(SDL_STATIC ON CACHE BOOL "")
add_subdirectory(third-party/SDL)

find_package(OpenGL REQUIRED)

add_subdirectory(third-party/stb)
add_subdirectory(third-party/glm)

SET(BUILD_SHARED_LIBS OFF CACHE BOOL "")
SET(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
add_subdirectory(third-party/assimp)

add_executable(${PROJECT_NAME} main.cc filesystem.cc program.cc texture.cc message_queue.h event.cc input.cc camera.cc)

target_link_libraries(${PROJECT_NAME} PUBLIC SDL2-static ${OPENGL_gl_LIBRARY} stb glm assimp)

if (APPLE)
    if (IOS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE GLES_SILENCE_DEPRECATION)
    else ()
        target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)
    endif ()
endif ()
