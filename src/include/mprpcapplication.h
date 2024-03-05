#pragma once
#include"mprpcconfig.h"
#include"mprpcchannel.h"
#include"mprpccontroller.h"
//mprpc框架的基础类
class MprpcApplication
{
    public:
    static void Init(int argc,char**argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();//获取配置
    private:
    static MprpcConfig m_config;//存放配置
    MprpcApplication(){};
    MprpcApplication(const MprpcApplication&)=delete;
    MprpcApplication(MprpcApplication&&)=delete;//设计成单例模式
};