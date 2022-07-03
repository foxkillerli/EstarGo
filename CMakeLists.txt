cmake_minimum_required(VERSION 3.10)

project(estar_go VERSION 1.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#--------------------------- PLATFORM SPECIFIC -------------------------------------------------------------------------

if(APPLE)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
endif()

configure_file(src/gtp.h.in src/gtp.h)
add_executable(estar_go
        src/gtp.cpp
        src/gtp.h
        main.cpp)
target_include_directories(estar_go PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(estar_go PUBLIC ${CMAKE_CURRENT_BINARY_DIR})