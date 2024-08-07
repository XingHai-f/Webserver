# epoll
epoll涉及的知识较多，这里仅对API和基础知识作介绍
## epoll_create函数
```C++
#include <sys/epoll.h>
int epoll_create(int size)
```
创建一个指示epoll内核事件表的文件描述符，该描述符将用作其他epoll系统调用的第一个参数，size不起作用。
