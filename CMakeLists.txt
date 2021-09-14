cmake_minimum_required(VERSION 3.16)
project(myMuduo)

set(CMAKE_CXX_STANDARD 14)

find_package(glog 0.6.0 REQUIRED)

add_library(myMuduo
        STATIC
        src/noncopyable.hpp
        src/TimeUtil.cpp
        src/TimeUtil.hpp
        src/EventLoop.cpp
        src/EventLoop.hpp
        src/Channel.cpp
        src/Channel.hpp
        src/Poller.cpp
        src/Poller.hpp
        src/Callbacks.hpp
        src/Timer.cpp
        src/Timer.hpp
        src/TimerId.hpp
        src/TimerQueue.cpp
        src/TimerQueue.hpp
        )
target_link_libraries(myMuduo glog::glog)

add_executable(test1 tests/test1.cpp)
target_link_libraries(test1 myMuduo)

add_executable(test2 tests/test2.cpp)
target_link_libraries(test2 myMuduo)

add_executable(test3 tests/test3.cpp)
target_link_libraries(test3 myMuduo)

add_executable(test4 tests/test4.cpp)
target_link_libraries(test4 myMuduo)
