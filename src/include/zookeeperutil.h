#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点 state默认0代表临时节点
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据参数指定的znode节点路径 获取znoe节点的值
    std::string GetData(const char *path);

private:
    // zk客户端句柄 通过句柄操作zkserver
    zhandle_t *m_zhandle;
};