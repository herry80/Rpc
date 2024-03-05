#pragma once

#include<unordered_map>
#include<string>
using namespace std;
//rpcserverip rpcserverport zookeeperip zookeeperport
//框架读取配置文件类
class MprpcConfig
{
    private:
    unordered_map<string,string>m_configMap;
    public:
    //赋值解析加载配置文件
    void LoadConfigFile(const char*config_file);
    //查询配置项信息
    string Load(const string&key);
    //去掉字符串前后的空格
    void Trim(string&src_buf);
};
