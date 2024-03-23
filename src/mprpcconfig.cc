#include "mprpcconfig.h"
#include <iostream>
#include <string>
// 负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    // 打开文件
    FILE *pf = fopen(config_file, "r");
    // 文件读取失败
    if (!pf)
    {
        std::cout << "file is not exists" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 文件读取成功
    // 需要处理 1.注释#  2.正确的配置项= 3.去掉开头的多余空格
    while (!feof(pf))
    {
        char buff[512] = {0};
        fgets(buff, 512, pf);

        std::string read_buff(buff);

        Trim(read_buff);
        // 判断#的注释
        if (read_buff[0] == '#' || read_buff.empty())
        {
            continue;
        }

        // 解析配置项
        int idx = read_buff.find('=');
        if (idx == -1)
        {
            // 配置项不合法
            continue;
        }

        std::string key;
        std::string value;
        key = read_buff.substr(0, idx);
        Trim(read_buff);
        int endidx = read_buff.find('\n', idx);
        value = read_buff.substr(idx + 1, endidx - idx - 1);
        m_configMap.insert({key, value});
    }
}
// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
        return "";
    else
        return it->second;
}

void MprpcConfig::Trim(std::string &str_buff)
{
    // 去掉字符串前面多余的空格
    int idx = str_buff.find_first_not_of(' ');
    if (idx != -1)
    {
        // 存在空格
        str_buff = str_buff.substr(idx, str_buff.size() - idx);
    }

    // 去掉字符串后面多余的空格
    idx = str_buff.find_last_not_of(' ');
    if (idx != -1)
    {
        // 存在空格
        str_buff = str_buff.substr(0, idx + 1);
    }
}