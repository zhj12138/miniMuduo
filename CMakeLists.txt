cmake_minimum_required(VERSION 3.16)
project(myMuduo)

set(CMAKE_CXX_STANDARD 14)

find_package(glog 0.6.0 REQUIRED)

add_library(myMuduo
        STATIC
        noncopyable.hpp
        TimeUtil.cpp
        TimeUtil.hpp
        EventLoop.cpp
        EventLoop.hpp
        Channel.cpp
        Channel.hpp
        Poller.cpp
        Poller.hpp
        Callbacks.hpp
        Timer.cpp
        Timer.hpp
        TimerId.hpp
        TimerQueue.cpp
        TimerQueue.hpp
        )
target_link_libraries(myMuduo glog::glog)

add_executable(test1 test1.cpp)
target_link_libraries(test1 myMuduo)

add_executable(test2 test2.cpp)
target_link_libraries(test2 myMuduo)

add_executable(test3 test3.cpp)
target_link_libraries(test3 myMuduo)

add_executable(test4 test4.cpp)
target_link_libraries(test4 myMuduo)
