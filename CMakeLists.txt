#cmake_minimum_required(VERSION 3.20)
#project(neon)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_C_STANDARD 17)
#
#set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
##set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
##set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
#add_subdirectory(third-party/glfw)
#
#find_package(OpenGL REQUIRED)
#
#add_subdirectory(third-party/stb)
#add_subdirectory(third-party/glm)
#add_subdirectory(third-party/tinycthread)
#
##SET(SDL_SHARED OFF CACHE BOOL "")
##SET(SDL_STATIC ON CACHE BOOL "")
#add_subdirectory(third-party/SDL)
#
##set(SOURCES main.cpp filesystem.cc shader.cc window.cc texture.cc event.cc renderer.cc)
#set(SOURCES main.cpp)
#add_executable(${PROJECT_NAME} ${SOURCES})
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${OPENGL_gl_LIBRARY} stb glm tinycthread SDL3-shared)
#if (APPLE)
#    if (IOS)
#        target_compile_definitions(${PROJECT_NAME} PRIVATE GLES_SILENCE_DEPRECATION)
#    else ()
#        target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)
#    endif ()
#endif ()

cmake_minimum_required(VERSION 3.23)

project(neon)

set(CMAKE_CXX_STANDARD 17)

SET(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "")
SET(SDL_SHARED OFF CACHE BOOL "")
SET(SDL_STATIC ON CACHE BOOL "")
add_subdirectory(third-party/SDL)

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} main.cc)

target_link_libraries(${PROJECT_NAME} PUBLIC SDL2-static ${OPENGL_gl_LIBRARY})
