cmake_minimum_required(VERSION 3.10)

project(estar_go VERSION 1.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread" )
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
LINK_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/include)

#--------------------------- PLATFORM SPECIFIC -------------------------------------------------------------------------

if(APPLE)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
endif()

configure_file(conf/Gtp.h.in include/Gtp.h)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src)
ADD_EXECUTABLE(estar_go main.cpp)
target_include_directories(estar_go PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(estar_go PUBLIC ${CMAKE_CURRENT_BINARY_DIR})