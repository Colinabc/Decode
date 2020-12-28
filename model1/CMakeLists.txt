cmake_minimum_required( VERSION 2.8 )
project(Model1)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 11)

find_package(gflags)
find_package(OpenCV REQUIRED)


include_directories(
       /usr/local/include/Anjian
       /usr/local/include/Anjian/third_party
       ${GFLAGS_INCLUDE_DIR}
       ${OpenCV_INCLUDE_DIRS} )

link_directories(/usr/local/lib)

link_libraries(  
       -pthread      
        gflags
        glog
       -lscheduler
       -ladapter
        ${OpenCV_LIBS})

add_executable(sample1 ./main.cpp)




