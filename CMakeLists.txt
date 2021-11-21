cmake_minimum_required(VERSION 3.16)
project(myMuduo)

set(CMAKE_CXX_STANDARD 17)
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
        src/EventLoopThreadPool.cpp
        src/EventLoopThreadPool.hpp
        src/Channel.cpp
        src/Channel.hpp
        src/Poller.cpp
        src/Poller.hpp
        src/EPoller.cpp
        src/EPoller.hpp
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
        src/Connector.cpp
        src/Connector.hpp
        src/TcpClient.cpp
        src/TcpClient.hpp
        src/http/HttpContext.cpp
        src/http/HttpResponse.cpp
        src/http/HttpResponse.hpp
        src/http/HttpServer.cpp
        src/http/HttpServer.hpp)
target_link_libraries(myMuduo glog::glog)

include_directories(src)

macro(add_muduo_executable target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} myMuduo)
endmacro()

link_libraries(myMuduo)

# Tests
add_muduo_executable(test1 tests/test1.cpp)
add_muduo_executable(test2 tests/test2.cpp)
add_muduo_executable(test3 tests/test3.cpp)
add_muduo_executable(test4 tests/test4.cpp)
add_muduo_executable(test5 tests/test5.cpp)
add_muduo_executable(test6 tests/test6.cpp)
add_muduo_executable(test7 tests/test7.cpp)
add_muduo_executable(test8 tests/test8.cpp)
add_muduo_executable(test9 tests/test9.cpp)
add_muduo_executable(test10 tests/test10.cpp)
add_muduo_executable(test11 tests/test11.cpp)
add_muduo_executable(test12 tests/test12.cpp)
add_muduo_executable(test13 tests/test13.cpp)

# benchmark
add_muduo_executable(mini_muduo_echo_server benchmark/mini_muduo_echo_server.cpp)

add_executable(asio_echo_server benchmark/asio_echo_server.cpp)
