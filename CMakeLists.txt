cmake_minimum_required(VERSION 3.22)
project(rubik)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include_directories(include ./ lib/glm lib/imgui lib/imgui/backends src/)
add_executable(rubik
        src/glad.c
        src/main.cpp src/common.h src/cube/RubiksCube.cpp src/cube/RubiksCube.h
        src/render/CubeRenderer.h src/render/CubeRenderer.cpp
        src/render/Camera.cpp src/render/Camera.h

        src/cube/solve/solver.cpp src/cube/solve/solver.h
        src/cube/solve/database.cpp src/cube/solve/database.h

        #ImGui
        lib/imgui/imgui.cpp lib/imgui/imgui.h
        lib/imgui/imgui_draw.cpp lib/imgui/imgui_widgets.cpp
        lib/imgui/imgui_tables.cpp lib/imgui/imgui_demo.cpp

        lib/imgui/backends/imgui_impl_glfw.cpp lib/imgui/backends/imgui_impl_glfw.h
        lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_opengl3.h
        src/util/easing.h src/util/RedundantMovePreventor.cpp src/util/RedundantMovePreventor.h src/cube/FastRubiksCube.cpp src/cube/FastRubiksCube.h src/render/CubeScanner.cpp src/render/CubeScanner.h src/render/CuberConnection.cpp src/render/CuberConnection.h src/cube/solve/kociemba.cpp src/cube/solve/kociemba.h src/cube/solve/solver_util.cpp src/cube/solve/solver_util.h)

# Add GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
add_subdirectory(lib/glfw)

# Add GLM
add_subdirectory(lib/glm)

# Add openGL
find_package(OpenGL REQUIRED)
#include_directories(${OPENGL_INCLUDE_DIR})
#find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui)



#Add OpenCV

# Configure OpenCV build before including it
set(BUILD_TESTS OFF CACHE BOOL "")
set(BUILD_PERF_TESTS OFF CACHE BOOL "")
set(BUILD_EXAMPLES OFF CACHE BOOL "")
set(BUILD_DOCS OFF CACHE BOOL "")
set(BUILD_opencv_apps OFF CACHE BOOL "")
set(BUILD_opencv_java OFF CACHE BOOL "")
set(BUILD_opencv_python OFF CACHE BOOL "")
set(BUILD_opencv_ts OFF CACHE BOOL "")
set(BUILD_LIST core,imgproc,highgui,imgcodecs,videoio CACHE STRING "OpenCV modules")

set(WITH_IPP OFF CACHE BOOL "")
set(WITH_TBB OFF CACHE BOOL "")
set(WITH_OPENMP OFF CACHE BOOL "")
set(WITH_CUDA OFF CACHE BOOL "")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build OpenCV statically" FORCE)
add_subdirectory(lib/opencv)
set(BUILD_SHARED_LIBS ON) # or don't set it globally at all

target_link_libraries(rubik PRIVATE
        glfw
        opencv_core
        opencv_imgproc
        opencv_highgui
        opencv_imgcodecs
        opencv_videoio
)

target_include_directories(rubik PRIVATE
        ${CMAKE_BINARY_DIR}/
        ${CMAKE_SOURCE_DIR}/lib/opencv/modules/core/include
        ${CMAKE_SOURCE_DIR}/lib/opencv/modules/imgproc/include
        ${CMAKE_SOURCE_DIR}/lib/opencv/modules/highgui/include
        ${CMAKE_SOURCE_DIR}/lib/opencv/modules/imgcodecs/include
        ${CMAKE_SOURCE_DIR}/lib/opencv/modules/videoio/include
        ${CMAKE_SOURCE_DIR}/lib/glfw/include
)

#option(BUILD_opencv_world "Build all OpenCV libs into a single library" OFF)

#add_subdirectory(lib/opencv EXCLUDE_FROM_ALL)
#include_directories(lib/opencv/modules/core/include)

#include_directories(C:/Users/Anatol/opencv/build/include)

#We want to target the right opencv_world lib
#if(CMAKE_BUILD_TYPE MATCHES Debug)
#    target_link_libraries(rubik C:/Users/Anatol/opencv/build/x64/vc16/lib/opencv_world490d.lib)
#else()
#    target_link_libraries(rubik C:/Users/Anatol/opencv/build/x64/vc16/lib/opencv_world490.lib)
#endif()