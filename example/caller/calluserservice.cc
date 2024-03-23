#include <iostream>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcchannel.h"
int main(int argc, char **argv)
{
    // 整个程序启动以后 想使用mprpc框架来享受rpc服务调用 一定需要先调用框架的初始化函数(只初始化一次)
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc放啊Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel()); // Rpcchannel

    // rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc方法的响应
    fixbug::LoginResponse response;

    // 发起rpc方法的调用 同步的rpc调用过程 MprpcChannel::callmethod
    stub.Login(nullptr, &request, &response, nullptr);
    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success : " << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error: " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc放啊Register

    // rpc方法的请求参数
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");

    // rpc方法的响应
    fixbug::RegisterResponse rsp;

    // 发起rpc方法的调用 同步的rpc调用过程 MprpcChannel::callmethod
    stub.Register(nullptr, &req, &rsp, nullptr);
    if (0 == rsp.result().errcode())
    {
        std::cout << "rpc register response success : " << rsp.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error: " << rsp.result().errmsg() << std::endl;
    }
    return 0;
}