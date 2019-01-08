cmake_minimum_required(VERSION 2.8)
#set(CMAKE_TOOLCHAIN_FILE "C:/Users/bae/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
project(icex)
message(status "boi ${CMAKE_TOOLCHAIN_FILE}")

# Glob files (make sure to clear cmake cache and rerun after adding/removing files)
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)
file(GLOB_RECURSE SHADERS shaders/*.vert shaders/*.frag shaders/*.comp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compile with c++14
set(CMAKE_CXX_STANDARD 11)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL -D_USE_MATH_DEFINES)
#add_definitions('--std=c++11')

# Preprocessor macros to locate resources and shader directory
add_definitions(-DRESOURCE_PATH="${CMAKE_SOURCE_DIR}/resources/")
add_definitions(-DSHADER_PATH="${CMAKE_SOURCE_DIR}/shaders/")

# Find all packages and check we found them
# If OpenCV and GLEW are not in default directories, set OpenCV_DIR and GLEW_DIR environment variables
#set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{OpenCV_DIR} $ENV{GLEW_DIR})
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenGL REQUIRED)
#find_package(GLFW REQUIRED)
find_package(GLEW)
find_path(EIGEN3_INCLUDE_DIR Eigen/Dense PATHS ENV EIGEN3_INCLUDE_DIR PATH_SUFFIXES eigen eigen3)
find_path(GLM_INCLUDE_DIR glm/glm.hpp PATHS ENV GLM_DIR GLM_LOCATION) 
find_package(OpenCV REQUIRED)

find_package(glfw3 CONFIG REQUIRED)

if (NOT EIGEN3_INCLUDE_DIR)
  message(FATAL_ERROR "Please point the environment variable EIGEN3_INCLUDE_DIR to the include directory of your Eigen3 installation.")
endif()

if (NOT GLM_INCLUDE_DIR)
    message(FATAL_ERROR "Please point the environment variable GLM_DIR or GLM_LOCATION to the include directory of your GLM installation.")
endif()

if (NOT GLEW_FOUND)
    if (DEFINED ENV{GLEW_DIR})
        if (WIN32)
            set(GLFW_LIBRARY "$ENV{GLEW_DIR}/lib/Release/Win32/glew32s.lib")
        else()
            set(GLFW_LIBRARY "$ENV{GLEW_DIR}/lib/libGLEW.a")
        endif()
    else()
        message(FATAL_ERROR "Can't find GLEW")
    endif()
endif()

# Include required directories and link libraries
include_directories(
    src/
    src/imgui
    ${EIGEN3_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${OPENGL_gl_LIBRARY}
    ${GLFW_LIBRARY}
    ${GLEW_LIBRARY}
    ${OpenCV_LIBS}
    glfw
)

if (APPLE)
    add_definitions(-std=c++14)
endif()
