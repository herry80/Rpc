#include"mprpcapplication.h"
#include<iostream>
#include<unistd.h>
#include<string>
using namespace std;
MprpcConfig MprpcApplication::m_config;//静态成员类外初始化。
void ShowArgsHelp()
{
        cout<<"format:command -i <congfigfile>"<<endl;
}
void MprpcApplication::Init(int argc,char**argv)
{
        if(argc<2)//没有传入参数
        {
                ShowArgsHelp();
                exit(EXIT_FAILURE);
        }
        int c=0;
        string config_file;
        while((c=getopt(argc,argv,"i:"))!=-1)
        {
                switch(c)
                {
                        case 'i':
                        config_file=optarg;
                        break;

                        case '?'://出现了不想要的参数，即不是i
                        ShowArgsHelp();
                        break;

                        case ':'://后边没跟参数
                        ShowArgsHelp();
                        exit(EXIT_FAILURE);
                        break;

                        default:
                        //ShowArgsHelp();
                        break;
                }
        }

        //开始加载配置文件
        m_config.LoadConfigFile(config_file.c_str());
        //cout<<"rpcserverip:"<<m_config.Load("rpcserverip")<<endl;
        //cout<<"rpcserverport:"<<m_config.Load("rpcserverport")<<endl;
        //cout<<"zookeeperip:"<<m_config.Load("zookeeperip")<<endl;
        //cout<<"zookeeperport:"<<m_config.Load("zookeeperport")<<endl;
}
MprpcApplication& MprpcApplication::GetInstance()
{
        static MprpcApplication app;
        return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
        return m_config;
}