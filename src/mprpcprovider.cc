#include "mprpcprovider.h"
#include "mprpcapplication.h"
#include "mprpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"
// 这里是框架提供给外部使用的 可以发布rpc方法的函数接口 所以函数参数使用父类Service
// 通过该方法将外部的方法注册到框架中
// 实现的主要内容是将   服务对象名字 服务对象 服务对象中方法名字 服务对象方法描述 保存到MprpcProvider中
void MprpcProvider::NotifyService(google::protobuf::Service *service)
{

    ServiceInfo service_info; // MprpcProvider中的自定义类型 存储名字及对应指针
    /*
        Service提供了
        const ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor* GetDescriptor();
        该方法用来描述服务
    */
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象service中方法的数量
    int methodCnt = pserviceDesc->method_count();
    LOG_INFO("service_name:%s", service_name.c_str());
    for (int i = 0; i < methodCnt; ++i)
    {
        // 获取服务对象指定下标的服务方法的描述(抽象描述)
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点 开始提供rpc远程网络调用服务
void MprpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = stoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport"));
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "MprpcProvider");
    // 绑定连接回调函数和消息读写回调函数 分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&MprpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&MprpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量
    /*
        muduo库是典型的reactor模式
        如果是1个线程 那就是io线程(建立用户连接)和工作线程是一个
        如果是4个线程 那就是io线程占一个 其他线程是工作线程
    */
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面， 让rpc client可以从zk上发现服务
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点  method_name为临时性节点
    for (auto &sp : m_serviceMap)
    {
        // /servicename
        std::string service_path = "/" + sp.first;      // 服务路径
        zkCli.Create(service_path.c_str(), nullptr, 0); // 将服务名在zk上创建一个节点 但没有携带数据
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name  /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + '/' + mp.first; // 方法路径
            char method_path_data[128] = {0};                        // 存放方法所在的ip和port
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL); // 创建临时节点 并携带ip和端口
        }
    }

    // std::cout << "Rpcserver start service at ip:" << ip << " port:" << port << std::endl;
    LOG_INFO("Rpcserver start service at ip:%s  port:%d", ip.c_str(), port);

    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}

void MprpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}

/*
在框架内部 MprpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name  method_name args  定义proto的message类型 进行数据头的序列化和反序列化
                                service_name method_name args_size
16UserServiceLoginzhang san123456

header_size(4个字节) + header_str + args_str
*/
// 已建立连接用户的读写事件回调 如果远程有一个rpc服务的调用请求 那么OnMessage方法就会响应
void MprpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp)
{
    // 网络上接收的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char *)&header_size, 4, 0); // 将recv_buf的前4个字节内容直接拷贝到head_size中

    // 根据header_size读取数据头的原始字符流 反序列化数据 得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size = 0;

    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列花失败
        // std::cout << "rpc_header_str:" << rpc_header_str << "parse error" << std::endl;
        LOG_INFO("rpc_header_str: %s parse error", rpc_header_str.c_str());
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "======================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "======================================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        // std::cout << service_name << " is not exits!" << std::endl;
        LOG_INFO("%s is not exits!", service_name.c_str());
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ':' << method_name << "is not exits!" << std::endl;
        LOG_INFO("%s : %s is not exits!", service_name.c_str(), method_name.c_str());
    }

    google::protobuf::Service *service = it->second.m_service;      // 获取service对象 new Uservice
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method的描述 Login

    // 生成rpc方法调用的  请求request 和响应response参数  Message是抽象类型 所有消息都继承Message
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        LOG_INFO("request parse error, content: %s", args_str.c_str());
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用 绑定要给Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<MprpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &MprpcProvider::SendRpcResponse,
                                                                                                 conn,
                                                                                                 response);

    // 在框架上根据远端rpc请求 调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    // controler 先不管  request和response是函数的请求和响应参数 done是回调函数 一般是把响应发回
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作 用于序列化rpc的响应和网络发送
void MprpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) // response序列化
    {
        // 序列化成功后  通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
    }
    else
    {
        // std::cout << "serilize response_str error!" << std::endl;
        LOG_INFO("serilize response_str error!");
    }

    conn->shutdown(); // 模拟http的短链接服务 由MprpcProvider主动断开连接
}