# epoll
epoll涉及的知识较多，这里仅对API和基础知识作介绍
## epoll_create函数
```C++
#include <sys/epoll.h>
int epoll_create(int size)
```
创建一个指示epoll内核事件表的文件描述符，该描述符将用作其他epoll系统调用的第一个参数，size不起作用。
## epoll_ctl函数
```C++
#include <sys/epoll.h>
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
```
该函数用于操作内核事件表监控的文件描述符上的事件：注册、修改、删除
* epfd：为epoll_creat的句柄
* op：表示动作，用3个宏来表示：
  * EPOLL_CTL_ADD (注册新的fd到epfd)，
  * EPOLL_CTL_MOD (修改已经注册的fd的监听事件)，
  * EPOLL_CTL_DEL (从epfd删除一个fd)；
* event：告诉内核需要监听的事件
上述event是epoll_event结构体指针类型，表示内核所监听的事件，具体定义如下：
