#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>             // 提供POSIX操作系统API，如文件操作、进程控制等
#include <signal.h>             // 信号处理，如中断、终止等
#include <sys/types.h>          // 定义数据类型，如pid_t、size_t等
#include <sys/epoll.h>          // 提供epoll相关函数，用于高效的I/O事件通知机制
#include <fcntl.h>              // 文件控制函数，如open、fcntl等
#include <sys/socket.h>         // 创建和操作套接字的函数
#include <netinet/in.h>         // 包含用于Internet地址族的结构和宏
#include <arpa/inet.h>          // 提供Internet操作函数，如IP地址转换
#include <assert.h>             // 提供assert宏，用于调试
#include <sys/stat.h>           // 文件状态操作，如获取文件信息
#include <string.h>             // 字符串操作函数
#include <pthread.h>            // 提供POSIX线程的创建和管理函数
#include <stdio.h>              // 输入输出函数，如printf、scanf
#include <stdlib.h>             // 常用库函数，如内存分配、进程控制等
#include <sys/mman.h>           // 内存映射相关函数，如mmap、munmap
#include <stdarg.h>             // 处理变参函数的宏，如va_start、va_arg
#include <errno.h>              // 提供系统错误代码
#include <sys/wait.h>           // 等待进程终止的函数，如wait、waitpid
#include <sys/uio.h>            // readv和writev函数，用于在单个操作中读取或写入多个非连续缓冲区
#include "../lock/locker.h"     // 引入自定义锁机制模块，包含互斥锁或条件变量的实现
#include "../CGImysql/sql_connection_pool.h" // 引入MySQL连接池模块，包含数据库连接管理功能

class http_conn
{
public:
    // 设置读取文件的名称m_real_file大小
    static const int FILENAME_LEN = 200;
    // 设置读缓冲区m_read_buf大小
    static const int READ_BUFFER_SIZE = 2048;
    // 设置写缓冲区m_write_buf大小
    static const int WRITE_BUFFER_SIZE = 1024;
    // 报文的请求方法，本项目只用到GET和POST
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    // 主状态机的状态
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    // 报文解析的结果
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    // 从状态机的状态
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    // 初始化套接字地址，函数内部会调用私有方法init
    void init(int sockfd,const sockaddr_in &addr);
    // 关闭http连接
    void close_conn(bool real_close=true);
    void process();
    // 读取浏览器端发来的全部数据
    bool read_once();
    // 响应报文写入函数
    bool write();
    sockaddr_in *get_address(){
        return &m_address;  
    }
    // 同步线程初始化数据库读取表
    void initmysql_result();
    // CGI使用线程池初始化数据库表
    void initresultFile(connection_pool *connPool);

 private:
    void init();
    // 从m_read_buf读取，并处理请求报文
    HTTP_CODE process_read();
    // 向m_write_buf写入响应报文数据
    bool process_write(HTTP_CODE ret);
    // 主状态机解析报文中的请求行数据
    HTTP_CODE parse_request_line(char *text);
    // 主状态机解析报文中的请求头数据
    HTTP_CODE parse_headers(char *text);
    // 主状态机解析报文中的请求内容
    HTTP_CODE parse_content(char *text);
    // 生成响应报文
    HTTP_CODE do_request();
 
    // m_start_line是已经解析的字符
    // get_line用于将指针向后偏移，指向未处理的字符
    char* get_line(){return m_read_buf+m_start_line;};

    // 从状态机读取一行，分析是请求报文的哪一部分
    LINE_STATUS parse_line();

    void unmap();
 
    // 根据响应报文格式，生成对应8个部分，以下函数均由do_request调用
    bool add_response(const char* format,...);
    bool add_content(const char* content);
    bool add_status_line(int status,const char* title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
 
public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;

private:
    int m_sockfd;
    sockaddr_in m_address;

    // 存储读取的请求报文数据
    char m_read_buf[READ_BUFFER_SIZE];
    // 缓冲区中m_read_buf中数据的最后一个字节的下一个位置
    int m_read_idx;
    // m_read_buf读取的位置m_checked_idx
    int m_checked_idx;
    // m_read_buf中已经解析的字符个数
    int m_start_line;

    // 存储发出的响应报文数据
    char m_write_buf[WRITE_BUFFER_SIZE];
    // 指示buffer中的长度
    int m_write_idx;

    // 主状态机的状态
    CHECK_STATE m_check_state;
    // 请求方法
    METHOD m_method;

    // 以下为解析请求报文中对应的6个变量
    // 存储读取文件的名称
    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;

    char *m_file_address;       // 读取服务器上的文件地址
    struct stat m_file_stat;
    struct iovec m_iv[2];       // io向量机制iovec
    int m_iv_count;
    int cgi;                    // 是否启用的POST
    char *m_string;             // 存储请求头数据
    int bytes_to_send;          // 剩余发送字节数
    int bytes_have_send;        // 已发送字节数
};

#endif
