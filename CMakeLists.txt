cmake_minimum_required(VERSION 3.10)

project(estar_go VERSION 1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -pthread -Wall " )

#--------------------------- PLATFORM SPECIFIC -------------------------------------------------------------------------

if(APPLE)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
endif()
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/external/pachi)

configure_file(conf/Gtp.h.in include/Gtp.h)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
ADD_EXECUTABLE(estar_go ${PROJECT_SOURCE_DIR} ../main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC pachi)
target_include_directories(estar_go PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(estar_go PUBLIC ${PROJECT_SOURCE_DIR}/external/pachi)

