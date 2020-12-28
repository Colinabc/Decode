cmake_minimum_required (VERSION 3.1)

# The entire project is under name "raintime"
project (DecodeServer
         LANGUAGES C CXX
         VERSION 1.0)

# Compilation FLAGS
# Use C++11 as the standard
set (CMAKE_CXX_STANDARD 11)

# 添加编译选项
#option(ENABLE_ARM "Enable arm" OFF)
#option(ENABLE_X86 "Enable x86" OFF)

#if(ENABLE_ARM)
#  add_definitions(-DARM_PLATFORM)
#endif(ENABLE_ARM)

#if(ENABLE_X86)
  #add_definitions(-DX86_PLATFORM)
#endif(ENABLE_X86)


file(GLOB POSE_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/*.c*" )

find_package(OpenCV REQUIRED)
find_package(gflags)
find_package(Protobuf)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/include/
  ${GLOG_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  )

link_directories(/usr/local/lib
  )

link_libraries(gflags ${OpenCV_LIBS}
  # glog
  glog
  # ffmpeg
  -lavformat
  -lavfilter
  -lavdevice
  -lswresample
  -lswscale
  -lavutil
  -lavcodec 
  # opencv
  ${OpenCV_LIBS}
  )

add_library(${PROJECT_NAME} SHARED ${POSE_SRCS})

add_executable(decoding ./decoding.cpp)
target_link_libraries(decoding ${PROJECT_NAME})
