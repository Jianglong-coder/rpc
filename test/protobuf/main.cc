#include "test.pb.h"
#include <iostream>
#include <string>

int main()
{
    //封装了login请求对象的数据
    fixbug::LoginRequest req;
    req.set_name("jiang long");
    req.set_pwd("123456");

    //对象数据序列化 -> char* 或者 string
    std::string send_str;
    if (req.SerializeToString(&send_str))
    {
        std::cout << send_str.c_str() << std::endl;
    }

    //从send_str反序列化一个login请求对象
    fixbug::LoginRequest reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}