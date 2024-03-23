#include "zookeeperutil.h"
#include "mprpcapplication.h" //获取zk的ip和端口号
#include <semaphore.h>
#include <iostream>

// 全局watcher观察器  zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT) // 如果回调的消息类型是和会话相关的消息类型
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh); // 获取上下文 从zh所代表的上下文或会话中获取一个信号量，并将其地址存储在sem指针中
            sem_post(sem);                             // 通知调用连接api的线程继续执行
        }
    }
}
ZkClient::ZkClient() : m_zhandle(nullptr) {}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄 释放资源 例如MySQL_Conn
    }
}

// 连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperprot");
    std::string connstr = host + ':' + port;

    /*
        zookeeper_mt:多线程版本的zookeeper客户端库 项目用的多线程版本 还有个单线程版本
        zookeeper的API客户端程序提供了三个线程
        1. 当前调用连接API的线程
        2. 网络I/O线程 去连接zookeeper服务端
        3. 回调线程 在与客户端连接成功后 会调用回调函数 这也是一个单独的线程
    */
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0); // 初始化句柄 在此处会起一个线程去连接zkserver
    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);             // 初始化信号量
    zoo_set_context(m_zhandle, &sem); // 设置上下文 保存信号量

    sem_wait(&sem); // 等待信号量
    std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    // 创建节点
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag = 0;
    flag = zoo_exists(m_zhandle, path, 0, nullptr); // 先判断节点是否存在 如果存在 就不重复创建了
    if (ZNONODE == flag)                            // 节点不存在
    {
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (ZOK == flag)
        {
            std::cout << "create node success! path:" << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "create node error! path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 传入参数指定的znode节点路径，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr); // 获取信息与返回值
    if (flag != ZOK)                                                     // 如果获取失败
    {
        std::cout << "get znode error... path:" << path << std::endl;
        return "";
    }
    else
    {
        // 获取成功
        return buffer;
    }
}
