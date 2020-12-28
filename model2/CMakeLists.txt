cmake_minimum_required( VERSION 2.8 )
project(Model2)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)

file(GLOB SRC_LIST "${PROJECT_SOURCE_DIR}/src/*.cpp")


find_package(gflags)
find_package(OpenCV REQUIRED)


include_directories(
                    /usr/local/include/Anjian
                    /usr/local/include/Anjian/third_party
                    ${PROJECT_SOURCE_DIR}/include
		    ${GFLAGS_INCLUDE_DIR}
                    ${OpenCV_INCLUDE_DIRS})
link_directories(/usr/local/lib)

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
  
  gflags
  glog
  -lscheduler
  -ladapter
)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

add_executable(sample2 ./main.cpp)
target_link_libraries(sample2 ${PROJECT_NAME})


