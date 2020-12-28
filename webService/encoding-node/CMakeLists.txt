cmake_minimum_required( VERSION 3.10 )

project(encoding)

IF (CMAKE_BUILD_TYPE STREQUAL Debug)  
    ADD_DEFINITIONS(-DDEBUG)  
ENDIF()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wl,--no-as-needed")

find_package(Protobuf REQUIRED)
find_package(GFLAGS)
find_package(OpenCV REQUIRED)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include/
    /usr/local/include/
)

link_directories(
    /usr/local/lib/
)

link_libraries(
    #ffmpeg_lib
    -lavformat
    -lavfilter
    -lavdevice
    -lswresample
    -lswscale
    -lavutil
    -lavcodec
)


file (GLOB server_src ${CMAKE_CURRENT_LIST_DIR}/src/*.c*)
file (GLOB main ${CMAKE_CURRENT_LIST_DIR}/encoding.cpp)


add_executable(encoding ${main} ${server_src})
target_link_libraries(encoding ${OpenCV_LIBS} gflags glog)
