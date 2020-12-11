cmake_minimum_required(VERSION 3.12)

project(gst-analyzer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt5Core)

include(FindPkgConfig)
if(NOT PKG_CONFIG_FOUND)
  message(FATAL_ERROR "pkg-config not found!" )
endif()
 
pkg_check_modules(GST REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4 gstreamer-video-1.0>=1.4 gstreamer-app-1.0>=1.4)

add_executable(${PROJECT_NAME} 
  main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GST)

