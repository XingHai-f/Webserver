# Webserver
Linux下的C++的轻量级Web服务器学习
* 使用 线程池 + 非阻塞socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现) 的并发模型
* 使用状态机解析HTTP请求报文，支持解析GET和POST请求
* 访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
* 实现同步/异步日志系统，记录服务器运行状态
* 经Webbench压力测试可以实现上万的并发连接数据交换

## 01线程同步机制封装类
定义了一组用于线程同步的工具类，使用了POSIX线程（pthread）和信号量（semaphore）。程序中包含三个类：sem、locker 和 cond，分别用于处理信号量、互斥锁和条件变量。这些类一起提供了一个基本的多线程同步机制，可以用于编写安全、可靠的多线程程序。
## 02半同步半反应堆线程池
实现了一个通用的线程池类 threadpool，用于管理多个线程并处理任务队列中的请求。


# 来源
[TinyWebServer](https://github.com/qinguoyi/TinyWebServer.git)
