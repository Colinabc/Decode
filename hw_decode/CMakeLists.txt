cmake_minimum_required( VERSION 2.8 )
project(HW_Decode)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)

file(GLOB SRC_LIST "${PROJECT_SOURCE_DIR}/src/*.cpp")

find_package(OpenCV REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/inc/
                    ${OpenCV_INCLUDE_DIRS})
link_directories(/usr/local/lib/)

link_libraries(
  -pthread

  # ffmpeg
  -lavformat
  -lavfilter
  -lavdevice
  -lswresample
  -lswscale
  -lavutil
  -lavcodec
  
  #opencv
  ${OpenCV_LIBS}
)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

add_executable(sample ./main.cpp)
target_link_libraries(sample ${PROJECT_NAME})


