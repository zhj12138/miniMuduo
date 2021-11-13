# miniMuduo

## 简介

miniMuduo是一个模仿muduo的简单网络库，基于Reactor模式。

## 核心类

### EventLoop

事件循环类，利用`thread_local`关键字让每个线程拥有一个独立的`EventLoop`对象。

* 核心方法是loop，在循环中不断地调用poll(或epoll_wait)：

  ```cpp
  void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
  
    while (!quit_) {
      activeChannels_.clear();
      pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
      for (const auto &channel: activeChannels_) {
        channel->handleEvent(pollReturnTime_);
      }
      doPendingFunctors();  // 调用pending functions
    }
  
    LOG(INFO) << "EventLoop " << this << " stop looping";
    looping_ = false;
  }
  ```
  
* 拥有一个TimerQueue对象(将在后面介绍)，给EventLoop添加了定时运行任务的功能。

* 借助TimerQueue对象，实现了以下功能：

  ```cpp
  TimerId runAt(const time_point &time, const TimerCallback &cb); // 在time时刻运行cb
  TimerId runAfter(double delay, const TimerCallback &cb); // delay秒之后运行cb
  TimerId runEvery(double interval, const TimerCallback &cb); // 没过interval秒运行一次cb
  ```

* 拥有一个wakeupfd和wakeup Channel对象，用于立即唤醒EventLoop。其中wakeupfd通过eventfd系统调用进行创建。

  唤醒原理：通过向wakeupfd写入数据，使得wakeupfd可读，所以poll调用就会立即返回。进而就可以立马调用活跃的Channel的回调函数，以及pending functions。

* 拥有一个方便的runInLoop函数：

  ```cpp
  void EventLoop::runInLoop(const Functor &cb) {
    if (isInLoopThread()) { // 在当前IO线程中调用runInLoop
      cb(); // 同步调用该函数
    } else {  // 在其他线程中调用runInLoop
      queueInLoop(cb);  // 将cb加入到EventLoop的pending functions中，然后尝试唤醒该EventLoop对象
    }
  }
  ```

### Channel

一个对`struct pollfd`的封装类：

* 包含fd, events, revents等成员，可以轻松的适应`poll`的接口和`epoll`的接口。

* 拥有`enableReading`、`enableWriting`等辅助函数来方便地设置对fd感兴趣的事件。

* 拥有`readCallback`，`writeCallback`，`errorCallback`，`closeCallback`这4个回调函数。

* 在每次poll(或epoll_wait)之后，EventLoop对象会调用每一个活跃的Channel的handleEvent函数，来根据poll(epoll_wait)的返回结果调用相应的回调函数：

  ```cpp
  void Channel::handleEvent(time_point receiveTime) {
    eventHandling_ = true;
    if (revents_ & POLLNVAL) {  // 文件描述符未打开
      LOG(WARNING) << "Channel::handle_event() POLLNVAL";
    }
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) { // 出现挂断
      LOG(WARNING) << "Channel::handle_event() POLLHUP";
      if (closeCallback_) {
        closeCallback_();
      }
    }
    if (revents_ & (POLLERR | POLLNVAL)) { // 出现错误
      if (errorCallback_) {
        errorCallback_();
      }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {  // 数据可读
      if (readCallback_) {
        readCallback_(receiveTime);
      }
    }
    if (revents_ & POLLOUT) { // 数据可写
      if (writeCallback_) {
        writeCallback_();
      }
    }
    eventHandling_ = false;
  }
  ```

### Poller

Poller是对`IO multiplexing`的封装：

* 核心方法是`poll`函数：

  * 首先调用linux的系统调用poll(或epoll_wait)。
  * 然后调用`fillActiveChannels`函数将poll(或epoll_wait)的调用结果填充到传入的`activeChannels`数组中。

  ```cpp
  time_point Poller::poll(int timeoutMs, Poller::ChannelVec *activeChannels) {
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    time_point now(get_now());
    if (numEvents > 0) {
      LOG(INFO) << numEvents << " events happended";
      fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
      LOG(INFO) << " nothing happended";
    } else {
      LOG(ERROR) << "Poller::poll()";
    }
    return now;
  }
  ```

### 时序图

(下图来源于《Linux多线程服务端编程》)

![image-20211113155409103](https://gitee.com/snow_zhao/img/raw/master/img/image-20211113155409103.png)

## 定时器

### Timer

定时器对象：

* 通过以下数据成员，记录了定时器的属性：expiration(过期时间), repeat(是否重复), interval(重复的时间间隔)。
* 有一个callback回调函数，可以通过Timer.run()函数调用该回调函数。(Timer.run会在时钟过期的时候被调用)。

### TimerQueue

定时器队列：

* 包含一个timerfd和一个Channel对象。通过linux的`timerfd`系列函数，将时间和文件描述符联系起来，进而可以方便的把定时功能嵌入到EventLoop中。

* addTimer函数用于添加新的Timer，cancel函数用于取消一个Timer。

* 核心函数是为TimerQueue中的Channel对象注册的readCallback函数：

  ```cpp
  void TimerQueue::handleRead(time_point _receiveTime) {
    loop_->assertInLoopThread();
    time_point now(get_now());
    readTimerfd(timerfd_, now); // 读取timerfd的数据
  
    EntryVec expired = getExpired(now); // 获取所有过期的Timer
  
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    for (auto &entry : expired) {
      entry.second->run();  // 调用Timer的回调函数
    }
    callingExpiredTimers_ = false;
  
    reset(expired, now);  // 重启重复的Timer, 更新下次过期的时间(内部通过timerfd_settime来实现)
  }
  ```

## 线程池

### EventLoopThreadPool

一个EventLoop线程的线程池。

* 可通过setThreadNum()来设置线程数。
* 可通过start()来创建线程，初始化线程池。
* 可通过getNextLoop()来获取下一个EventLoop。(通过轮询来实现)。

## TCP网络库

### Socket

对C语言socket的封装。

### Buffer

缓冲区对象：

* 可以使用append函数往缓冲区尾端写数据。
* 可以使用prepend函数向缓冲区首部写数据。
* 可以使用retrieve函数读取数据。
* 有一个便捷的retrieveAllAsString函数可以读取所有可读取的数据，并放到string中。

![image-20211113205158402](https://gitee.com/snow_zhao/img/raw/master/img/image-20211113205158402.png)

### Acceptor

Acceptor类用于accept新的TCP连接：

* Acceptor中包含了一个Socket对象和一个accept Channel。

* Acceptor对象可以注册一个newConnectionCallback回调函数。

* 核心函数是为accept Channel注册的readCallback回调函数：

  ```cpp
  void Acceptor::handleRead(time_point _receiveTime) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0) {
      if (newConectionCallback_) {
        newConectionCallback_(connfd, peerAddr);
      } else {
        sockets::close(connfd);
      }
    }
  }
  ```

### TCPServer

TCPServer类用于管理通过accept获得的TcpConnection，直接供用户使用：

* TCPServer包含一个Acceptor对象，用于accept新的TCP连接。

* 包含了一个EventLoopThreadPool对象，通过轮流给各个线程添加任务，进而均衡各个线程的EventLoop中的任务。可以通过TCPServer.setThreadNum方法来设置线程池的线程数。

* 可以注册connectionCallback, messageCallback, writeCompleteCallback这等回调函数。

* 通过调用start()来创建线程池，并开始accept：

  ```cpp
  void TcpServer::start() {
    if (!started_.exchange(true)) {
      threadPool_->start();
      assert(!acceptor_->listenning());
      loop_->runInLoop([acceptor = acceptor_.get()] { acceptor->listen(); });
    }
  }
  ```

* 核心函数是为Acceptor注册的newConnectionCallback回调函数：

  ```cpp
  void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG(INFO) << "TcpServer::newConnection [" << name_
              << "] - new connection [" << connName
              << "] from " << peerAddr.toHostPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    EventLoop *ioLoop = threadPool_->getNextLoop();
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback([this](auto &&PH1) { removeConnection(std::forward<decltype(PH1)>(PH1)); });
    ioLoop->runInLoop([conn] { conn->connectEstablished(); });
  }
  ```

### TcpConnection

表示一次TCP连接：

* 包含一个Channel对象(在构造函数中创建)，同时会在构造函数中为Channel注册回调函数：

  ```cpp
  channel_->setReadCallback([this](auto &&PH1) { handleRead(std::forward<decltype(PH1)>(PH1)); });
  channel_->setWriteCallback([this] { handleWrite(); });
  channel_->setCloseCallback([this] { handleClose(); });
  channel_->setErrorCallback([this] { handleError(); });
  ```

* 允许注册connectionCallback、messageCallback、writeCompleteCallback、closeCallback等回调函数。

* 其中Channel的readCallback回调函数如下：

  ```cpp
  void TcpConnection::handleRead(time_point receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
      messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
      handleClose();
    } else {
      handleError();
    }
  }
  ```

* Channel的writeCallback回调函数如下：

  ```cpp
  void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
      ssize_t n = ::write(channel_->fd(),
                          outputBuffer_.peek(),
                          outputBuffer_.readableBytes());
      if (n > 0) {
        outputBuffer_.retrieve(n);
        if (outputBuffer_.readableBytes() == 0) {
          channel_->disableWriting();
          if (writeCompleteCallback_) {
            loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
          }
          if (state_ == kDisconnecting) {
            shutdownInLoop();
          }
        } else {
          LOG(INFO) << "I am going to write more data";
        }
      } else {
        LOG(ERROR) << "TcpConnection::handleWrite";
      }
    } else {
      LOG(INFO) << "Connection is down, no more writing";
    }
  }
  ```

* 可以调用send函数来发送数据。send函数首先尝试直接发送数据，如果一次发送完毕，就不会调用writeCallback。但是如果只发送了部分数据，就放到outputBuffer中，并且关注writable时间，以后在handleWrite中发送剩余的数据。send的代码如下：

  ```cpp
  void TcpConnection::send(const std::string &message) {
    if (state_ == kConnected) {
      if (loop_->isInLoopThread()) {
        sendInLoop(message);
      } else {
        loop_->runInLoop([this, message] { sendInLoop(message); });
      }
    }
  }
  ```

  ```cpp
  void TcpConnection::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
      nwrote = ::write(channel_->fd(), message.data(), message.size());
      if (nwrote >= 0) {
        if (implicit_cast<size_t>(nwrote) < message.size()) {
          LOG(INFO) << "I am going to write more data";
        } else if (writeCompleteCallback_) {
          loop_->queueInLoop([this] { writeCompleteCallback_(shared_from_this()); });
        }
      } else {
        nwrote = 0;
        if (errno != EWOULDBLOCK) {
          LOG(ERROR) << "TcpConnection::sendInLoop";
        }
      }
    }
    assert(nwrote >= 0);
    if (implicit_cast<size_t>(nwrote) < message.size()) { // 还没有发送完，放到outputBuffer中
      outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
      if (!channel_->isWriting()) {
        channel_->enableWriting();
      }
    }
  }
  ```

### Connector

Connector通过调用connect来连接服务器。

* Connector包含一个Channel对象：

  ```cpp
  channel_->setWriteCallback([this] { handleWrite(); });
  channel_->setErrorCallback([this] { handleError(); });
  ```

  并且可以注册一个connectionCallback回调函数。

  其中handleWrite的代码如下：

  ```cpp
  void Connector::handleWrite() {
    LOG(INFO) << "Connector::handleWrite " << state_;
    if (state_ == kConnecting) {
      int sockfd = removeAndResetChannel();
      int err = sockets::getSocketError(sockfd);
      if (err) {
        LOG(WARNING) << "Connector::handleWrite - SO_ERROR = "
                     << err << " " << strerror(err);
        retry(sockfd);
      } else if (sockets::isSelfConnect(sockfd)) {  // 自连接
        LOG(WARNING) << "Connector::handleWrite = Self Connect";
        retry(sockfd);
      } else {
        setState(kConnected);
        if (connect_) {
          newConnectionCallback_(sockfd);
        } else {
          sockets::close(sockfd);
        }
      }
    } else {
      assert(state_ == kDisconnected);
    }
  }
  ```

* 可以通过start函数开始进行connect。调用stop来停止连接。

### TcpClient

* TcpClient包含一个Connector来进行connect。

* 还可以注册connectionCallback, messageCallback, writeCallback等回调函数。

* 我们可以调用TcpClient.connect()函数来进行连接, TcpClient.disconnect()函数来关闭连接。

* TcpClient的核心代码是为Connector注册的connectionCallback回调函数：

  ```cpp
  void TcpClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = buf;
  
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback([this](const TcpConnectionPtr &conn) { removeConnection(conn); });
    {
      std::lock_guard<std::mutex> lg(mutex_);
      connection_ = conn;
    }
    conn->connectEstablished();
  }
  ```

  

