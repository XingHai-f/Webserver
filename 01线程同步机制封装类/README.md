# RAII
*  RAII全称是“Resource Acquisition is Initialization”，直译过来是“资源获取即初始化”.
*  在构造函数中申请分配资源，在析构函数中释放资源。因为C++的语言机制保证了，当一个对象创建的时候，自动调用构造函数，当对象超出作用域的时候会自动调用析构函数。所以，在RAII的指导下，我们应该使用类来管理资源，将资源和对象的生命周期绑定
*  RAII的核心思想是将资源或者状态与对象的生命周期绑定，通过C++的语言机制，实现资源和状态的安全管理,智能指针是RAII最好的例子
# 信号量
信号量是一种特殊的变量，它只能取自然数值并且只支持两种操作：等待(P)和信号(V).假设有信号量SV，对其的P、V操作如下：
*  P，如果SV的值大于0，则将其减一；若SV的值为0，则挂起执行
*  V，如果有其他进行因为等待SV而挂起，则唤醒；若没有，则将SV值加一
信号量的取值可以是任何自然数，最常用的，最简单的信号量是二进制信号量，只有0和1两个值.
## sem类
```C++
  int sem_init(sem_t *sem, int pshared, unsigned int value);
```
* sem_init函数：初始化一个匿名的信号量
* sem：指定了要初始化的信号量的地址；pshared：0表示多线程，非0表示多进程；value：指定了信号量的初始值
```C++
  int sem_destroy(sem_t *sem);
```
* sem_destory函数：用于销毁信号量
* sem：指定要销毁的匿名信号量的地址
```C++
  int sem_wait(sem_t *sem);
```
* sem_wait函数：如果信号量的值大于零，则将其值减一并立即返回。如果信号量的值为零，则sem_wait阻塞，直到信号量的值大于零。
* sem: 指向要操作的信号量对象的指针。
```C++
i  nt sem_post(sem_t *sem);
```
* sem_post函数：用于将信号量的值增加1。如果有任何线程因为信号量值为0，而阻塞在sem_wait或sem_trywait调用上，sem_post会唤醒其中的一个线程，使其继续执行。
* sem: 指向需要发布的信号量对象的指针。
以上，成功返回0，失败返回errno

# 互斥量 
互斥锁,也成互斥量,可以保护关键代码段,以确保独占式访问.当进入关键代码段,获得互斥锁将其加锁;离开关键代码段,唤醒等待该互斥锁的线程。（通俗解释用于防止多个线程同时访问共享资源。互斥量通过锁和解锁操作来控制对共享资源的访问，确保同一时刻只有一个线程能够访问该资源。）
## locker类 
```C++
  int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
```
* pthread_mutex_init函数：用于初始化互斥锁
* mutex: 指向需要初始化的互斥锁对象的指针；attr: 指向互斥锁属性对象的指针。如果使用默认属性，可以传递NULL。
```C++
  int pthread_mutex_destroy(pthread_mutex_t *mutex);
``` 
* pthread_mutex_destory函数：用于销毁互斥锁。
* mutex: 指向需要销毁的互斥锁对象的指针
```C++
  int pthread_mutex_destroy(pthread_mutex_t *mutex);
```
* pthread_mutex_lock函数：用于对互斥锁进行加锁操作。互斥锁是一种同步机制，用于防止多个线程同时进入临界区，从而避免数据竞争和不一致性。
* mutex: 指向需要加锁的互斥锁对象的指针。
```C++
  int pthread_mutex_unlock(pthread_mutex_t *mutex);
```
* pthread_mutex_unlock函数：用于解锁一个互斥锁（mutex）。当一个线程持有互斥锁并完成临界区中的操作后，需要调用这个函数释放锁，以便其他线程可以获得锁并进入临界区。
* mutex: 指向需要解锁的互斥锁对象的指针。
以上，成功返回0，失败返回errno

# 条件变量
条件变量提供了一种线程间的通知机制,当某个共享数据达到某个值时,唤醒等待这个共享数据的线程.
##  cond类
```C++
  int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
```
* pthread_cond_init函数：用于初始化条件变量
* cond: 指向需要初始化的条件变量对象的指针。attr: 指向条件变量属性对象的指针。如果使用默认属性，可以传递NULL。
```C++
int pthread_cond_destroy(pthread_cond_t *cond);
```
* pthread_cond_destory函数：销毁条件变量
* cond: 指向需要销毁的条件变量对象的指针。
```C++
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
```
* pthread_cond_timedwait函数:用于条件变量的函数，它允许线程等待一个特定的时间段，直到条件变量被信号唤醒或时间超时。
* cond：指向条件变量的指针。mutex：指向互斥锁的指针。条件变量需要与互斥锁配合使用，以避免竞态条件。abstime：指定绝对时间的timespec结构体，表示等待的超时时间。该时间是相对于1970年1月1日00:00:00 UTC 的时间。
```C++
int pthread_cond_signal(pthread_cond_t *cond);
```
* pthread_cond_signal：用来唤醒一个正在等待条件变量的线程。如果有多个线程在等待同一个条件变量，则pthread_cond_signal会唤醒其中一个线程。
* cond：指向条件变量的指针。
```C++
int pthread_cond_broadcast(pthread_cond_t *cond);
```
* pthread_cond_broadcast函数：用于唤醒所有等待指定条件变量的线程
* cond: 指向条件变量对象的指针。
```C++
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
```
* pthread_cond_wait函数：用于等待目标条件变量.该函数调用时需要传入 mutex参数(加锁的互斥锁) ,函数执行时,先把调用线程放入条件变量的请求队列,然后将互斥锁mutex解锁,当函数成功返回为0时,互斥锁会再次被锁上. 也就是说函数内部会有一次解锁和加锁操作.
* cond: 指向条件变量对象的指针。mutex: 指向与条件变量关联的互斥锁对象的指针。

# 功能
锁机制的功能
* 实现多线程同步，通过锁机制，确保任一时刻只能有一个线程能进入关键代码段.
# 封装的功能
类中主要是Linux下三种锁进行封装，将锁的创建于销毁函数封装在类的构造与析构函数中，实现RAII机制
# 术语解释
## 阻塞
“阻塞”（blocking）是指一个线程暂停执行并等待某个条件满足的状态。当一个线程调用某个阻塞操作时，例如 sem_wait，它会进入一种等待状态，直到条件满足或者事件发生，它才会继续执行。
1. 阻塞状态：
* 当一个线程进入阻塞状态，它会暂停执行，不会占用 CPU 时间片。这意味着 CPU 可以调度其他线程或进程执行，而不是让阻塞的线程继续运行。
* 在阻塞期间，线程处于一种“睡眠”状态，直到特定条件满足时被唤醒。
2. 唤醒：
* 阻塞的线程在条件满足时会被唤醒。例如，调用 sem_wait 阻塞的线程会在信号量的值变为大于零时被唤醒。
* 被唤醒后，线程会重新进入就绪状态（ready state），等待 CPU 调度它执行
