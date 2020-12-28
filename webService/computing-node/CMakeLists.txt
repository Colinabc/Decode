cmake_minimum_required( VERSION 3.10 )

project(computing)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)  
    ADD_DEFINITIONS(-DDEBUG)  
ENDIF()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wl,--no-as-needed")

find_package(Protobuf REQUIRED)
find_package(GFLAGS)
find_package(OpenCV REQUIRED)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include/
    /usr/local/cr_sdk/include/
    /usr/local/cr_sdk/include/adapter/
    /usr/local/cr_sdk/include/third_party/
    /usr/local/include/
)

link_directories(
    /usr/local/cr_sdk/lib/
    /usr/local/lib/
)

link_libraries(
    libadapter.so
    libscheduler.so
    libyolo.so
    libajutils.so
    libimgrunner.so
)


file (GLOB server_src ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file (GLOB main ${CMAKE_CURRENT_LIST_DIR}/computing.cpp)


add_executable(computing ${main} ${server_src})
target_link_libraries(computing ${OpenCV_LIBS} RbRuntime gflags pthread protobuf glog rainman libstdc++.a)
