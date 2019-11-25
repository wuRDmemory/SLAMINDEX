cmake_minimum_required(VERSION 2.8.11)

project(Detector)

set(CMAKE_CXX_STANDARD 11)

include_directories(/usr/include/eigen3)
include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(OpenCV REQUIRED)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(Detector main.cc ${SOURCES})
target_link_libraries(Detector ${OpenCV_LIBS})
