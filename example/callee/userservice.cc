#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcprovider.h"

/*
UserService 原来是一个本地服务 提供了两个进程内的本地方法 Login和GetFriendLists
*/

// class UserService
// {
// public:
//     bool Login(std::string name, std::string pwd)
//     {
//         std::cout << name << std::endl;
//         std::cout << pwd << std::endl;
//     }
// };

/*
现在想把上面的UserService变成rpc方法
*/
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端(rpc服务提供者)
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing register service: Register" << std::endl;
        std::cout << "id:" << id << " name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }
    /*
        首先远端发起rpc请求到这 然后由这段的rpc框架先接收到到这个请求
        然后根据方法名和参数 匹配到这个Login方法
        从request请求中拿出数据
        然后处理完之后
        填写相应的响应reponse
        然后执行回调 done(将rpc方法执行完的reponse塞给框架 由框架序列化 给网络部分将相应发送回去)
    */
    /*
        重写基类UserServiceRpc的虚函数，以下方法由框架直接调用
        1.caller远程调用者发起远程调用请求Login(LoginRequest)=>muduo=>callee
        2.callee发现远程请求调用Login(LoginRequest)=>交付给下面这个重写的方法

        1.从LoginRequest获取参数的值
        2.执行本地服务Login，并获取返回值
        3.用上面的返回值填写LoginResponse
        4.一个回调，把LoginResonse发送给发起RPC服务的主机
    */
    virtual void Login(::google::protobuf::RpcController *controller,
                       const ::fixbug::LoginRequest *request,
                       ::fixbug::LoginResponse *response,
                       ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数 LoginRequest 应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入 包括错误码 错误消息 返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调函数
        done->Run();
    }
    virtual void Register(::google::protobuf::RpcController *controller,
                          const ::fixbug::RegisterRequest *request,
                          ::fixbug::RegisterResponse *response,
                          ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数 RegisterRequest 应用获取相应数据做本地业务
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool register_result = Register(id, name, pwd);

        // 把响应写入 包括错误码 错误消息 返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(register_result);

        // 执行回调函数
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作
    // 运行时 provider -i config.conf  读配置文件 读相关的IP和端口号
    /*
        argc 表示传递给程序的命令行参数 至少为1 第一个参数始终是程序的名称
        argv 指向字符串数组的指针 每个字符串表示一个命令行参数 第一个元素argv[0]是程序名称
        provider -i config.conf
        第一个参数是provider
        第二个参数是-i
        第三个参数是config.conf
    */
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象 把UserService对象发布到rpc节点上
    MprpcProvider provider;
    provider.NotifyService(new UserService());
    // provider.NotifyService(new ProductService());

    // 启动一个rpc服务发布节点  Run以后 进程进入阻塞装填 等待远程的rpc调用请求
    provider.Run();
    return 0;
}