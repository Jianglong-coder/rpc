#include <iostream>
#include <string>
#include <vector>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "mprpcprovider.h"
#include "logger.h"
class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t user_id)
    {
        std::cout << "do GetFriendsList service! userid: " << user_id << std::endl;
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("wang shuo");
        return vec;
    }
    // 重写基类方法
    virtual void GetFriendsList(::google::protobuf::RpcController *controller,
                                const ::fixbug::GetFriendsListRequest *request,
                                ::fixbug::GetFriendsListResponse *response,
                                ::google::protobuf::Closure *done)
    {
        uint32_t userid = request->userid();
        std::vector<std::string> friendsList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string &name : friendsList)
        {
            std::string *p = response->add_friends();
            *p = name;
        }
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
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象 把UserService对象发布到rpc节点上
    MprpcProvider provider;
    provider.NotifyService(new FriendService());
    // provider.NotifyService(new ProductService());

    // 启动一个rpc服务发布节点  Run以后 进程进入阻塞装填 等待远程的rpc调用请求
    provider.Run();
    return 0;
}