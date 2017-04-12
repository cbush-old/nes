cmake_minimum_required(VERSION 3.4)

project(nes)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall")

set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(RES_DIR ${PROJECT_SOURCE_DIR}/res)
set(SHADERS_DIR ${PROJECT_SOURCE_DIR}/shaders)

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.h" "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE SRC_SHADERS "${SHADERS_DIR}/*.glsl")
set(SRC_FILES ${SRC_FILES} ${SRC_SHADERS})

set(INCLUDE_DIRS
  ${SRC_DIR}
  ${LIB_DIR}/glm
  ${LIB_DIR}/SDL-mirror/include
)

find_package(OpenGL REQUIRED)

set(SYSTEM_INCLUDE_DIRS
  ${OPENGL_INCLUDE_DIR}/Headers
  ${OPENGL_INCLUDE_DIR}/Headers/OpenGL
  /opt/local/include # libsamplerate
)

# GLM
file(GLOB_RECURSE LIB_GLM_INCLUDE "${LIB_DIR}/glm/*.hpp")

# SDL2
add_subdirectory("${LIB_DIR}/SDL-mirror")

source_group(mappers        REGULAR_EXPRESSION "${SRC_DIR}/mappers/.*")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -Werror")

include_directories(${INCLUDE_DIRS})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRS})

add_library(nes STATIC ${SRC_FILES})

message(STATUS "OpenGL headers: ${OPENGL_INCLUDE_DIR}")

set(GAME_RES_DIR ${PROJECT_SOURCE_DIR}/game/res)

add_executable(main main.cpp)

find_library(IMLIB2 Imlib2 /opt/local/lib)
find_library(SAMPLERATE samplerate /opt/local/lib)

target_link_libraries(main
  nes
  SDL2-static
  ${IMLIB2}
  ${SAMPLERATE}
  ${OPENGL_LIBRARIES}
)

add_custom_command(TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RES_DIR} $<TARGET_FILE_DIR:main>/res)

add_custom_command(TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADERS_DIR} $<TARGET_FILE_DIR:main>/res)

