cmake_minimum_required(VERSION 3.16)
project(myMuduo)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread")

find_package(glog 0.6.0 REQUIRED)

add_library(myMuduo
        STATIC
        src/noncopyable.hpp
        src/TimeUtil.cpp
        src/TimeUtil.hpp
        src/Cast.hpp
        src/Buffer.cpp
        src/Buffer.hpp
        src/Thread.cpp
        src/Thread.hpp
        src/EventLoop.cpp
        src/EventLoop.hpp
        src/EventLoopThread.cpp
        src/EventLoopThread.hpp
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
        src/SocketsOps.cpp
        src/SocketsOps.hpp
        src/InetAddress.cpp
        src/InetAddress.hpp
        src/Socket.cpp
        src/Socket.hpp
        src/Acceptor.cpp
        src/Acceptor.hpp
        src/TcpConnection.cpp
        src/TcpConnection.hpp
        src/TcpServer.cpp
        src/TcpServer.hpp
        )
target_link_libraries(myMuduo glog::glog)

include_directories(src)

add_executable(test1 tests/test1.cpp)
target_link_libraries(test1 myMuduo)

add_executable(test2 tests/test2.cpp)
target_link_libraries(test2 myMuduo)

add_executable(test3 tests/test3.cpp)
target_link_libraries(test3 myMuduo)

add_executable(test4 tests/test4.cpp)
target_link_libraries(test4 myMuduo)

add_executable(testThread tests/testThread.cpp src/TimeUtil.cpp)
target_link_libraries(testThread pthread)

add_executable(test5 tests/test5.cpp)
target_link_libraries(test5 myMuduo)

add_executable(test6 tests/test6.cpp)
target_link_libraries(test6 myMuduo)

add_executable(test7 tests/test7.cpp)
target_link_libraries(test7 myMuduo)

add_executable(test8 tests/test8.cpp)
target_link_libraries(test8 myMuduo)

add_executable(test9 tests/test9.cpp)
target_link_libraries(test9 myMuduo)

add_executable(test10 tests/test10.cpp)
target_link_libraries(test10 myMuduo)

add_executable(test11 tests/test11.cpp)
target_link_libraries(test11 myMuduo)
