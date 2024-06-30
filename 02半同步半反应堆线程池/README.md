# pthread_create陷阱
函数原型
```C++
#include <pthread.h>
int pthread_create (pthread_t *thread_tid,               //返回新生成的线程的id
                   const pthread_attr_t *attr,           //指向线程属性的指针,通常设置为NULL
                   void * (*start_routine) (void *),     //处理线程函数的地址
                   void *arg);                           //start_routine()中的参数
```
函数原型中的第三个参数，为函数指针，指向处理线程函数的地址。该函数，要求为静态函数。如果处理线程函数为类成员函数时，需要将其设置为静态成员函数。

## this指针
pthread_create的函数原型中第三个参数的类型为函数指针，指向的线程处理函数参数类型为(void *),若线程函数为类成员函数，则this指针会作为默认的参数被传进函数中，从而和线程函数参数(void*)不能匹配，不能通过编译。
* 静态成员函数就没有这个问题，里面没有this指针。

# 线程池
* 空间换时间,浪费服务器的硬件资源,换取运行效率.
* 池是一组资源的集合,这组资源在服务器启动之初就被完全创建好并初始化,这称为静态资源。
* 当服务器进入正式运行阶段,开始处理客户请求的时候,如果它需要相关的资源,可以直接从池中获取,无需动态分配。
* 当服务器处理完一个客户连接后,可以把相关的资源放回池中,无需执行系统调用释放资源。
* 线程池的设计模式为半同步/半反应堆，其中反应堆具体为Proactor事件处理模式。
* 具体的，主线程为异步线程，负责监听文件描述符，接收socket新连接，若当前监听的socket发生了读写事件，然后将任务插入到请求队列。工作线程从请求队列中取出任务，完成读写数据的处理。
## 线程池类定义
线程处理函数和运行函数设置为私有属性。
```C++
 template<typename T>
 class threadpool{
     public:
       threadpool(connection_pool *connPool, int thread_number = 8, int max_request = 10000);
       // connPool是数据库连接池指针，thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量
       ~threadpool();

        bool append(T* request); // 向请求队列中插入任务请求

      private:
        static void *worker(void *arg); // 工作线程运行的函数
        void run(); // 不断从工作队列中取出任务并执行之

      private:
        int m_thread_number; // 线程池中的线程数
        int m_max_requests; // 请求队列中允许的最大请求数
        pthread_t *m_threads; // 描述线程池的数组，其大小为m_thread_number
        std::list<T *>m_workqueue; // 请求队列
        locker m_queuelocker; // 保护请求队列的互斥锁
        sem m_queuestat; // 是否有任务需要处理
        bool m_stop; // 是否结束线程
        connection_pool *m_connPool; // 数据库连接池指针
};
```
## 线程池创建与回收
* 构造函数中创建线程池,pthread_create函数中将类的对象作为参数传递给静态函数(worker),在静态函数中引用这个对象,并调用其动态方法(run)。
* 具体的，类对象传递时用this指针，传递给静态函数后，将其转换为线程池类，并调用私有成员函数run。
```C++
template<typename T>
threadpool<T>::threadpool( connection_pool *connPool, int thread_number, int max_requests)
      : m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(NULL),m_connPool(connPool)
{
    if(thread_number<=0||max_requests<=0)
        throw std::exception();
 
    //线程id初始化
    m_threads=new pthread_t[m_thread_number];
    if(!m_threads)
        throw std::exception();

    for(int i=0;i<thread_number;++i)
    {
        //循环创建线程，并将工作线程按要求进行运行
        if(pthread_create(m_threads+i,NULL,worker,this)!=0)
        {
            delete [] m_threads;
            throw std::exception();
        }

        //将线程进行分离后，不用单独对工作线程进行回收（线程在结束时能够自动释放资源）
        if(pthread_detach(m_threads[i])) // pthread_detach函数将新创建的线程设置为分离状态
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}
```
# 向请求队列中添加任务
通过list容器创建请求队列，向队列中添加时，通过互斥锁保证线程安全，添加完成后通过信号量提醒有任务要处理，最后注意线程同步。
```C++
template<typename T>
bool threadpool<T>::append(T* request)
{
    m_queuelocker.lock();
 
    //根据硬件，预先设置请求队列的最大值
    if(m_workqueue.size()>m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }

    //添加任务
    m_workqueue.push_back(request);
    m_queuelocker.unlock();

    //信号量提醒有任务要处理
    m_queuestat.post();
    return true;
}
```
# 线程处理函数
内部访问私有成员函数run，完成线程处理要求。
```C++
template<typename T>
void* threadpool<T>::worker(void* arg){
    //将参数强转为线程池类，调用成员方法
    threadpool* pool=(threadpool*)arg;
    pool->run();
    return pool;
}
```
## run执行任务
主要实现，工作线程从请求队列中取出某个任务进行处理，注意线程同步。
```C++
template<typename T>
void threadpool<T>::run()
{
     while(!m_stop)
     {    
         //信号量等待
         m_queuestat.wait();

        //被唤醒后先加互斥锁
        m_queuelocker.lock();
        if(m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }

        //从请求队列中取出第一个任务
        //将任务从请求队列删除
        T* request=m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request)
            continue;
  
        //从连接池中取出一个数据库连接
        request->mysql = m_connPool->GetConnection();

        //process(模板类中的方法,这里是http类)进行处理
        request->process();

        //将数据库连接放回连接池
        m_connPool->ReleaseConnection(request->mysql);
    }
}
```
