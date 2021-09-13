cmake_minimum_required(VERSION 3.16)
project(myMuduo)

set(CMAKE_CXX_STANDARD 14)

find_package(glog 0.6.0 REQUIRED)

add_executable(myMuduo EventLoop.cpp)
target_link_libraries(myMuduo glog::glog)
