#include"include/mprpcconfig.h"
#include<iostream>
#include<string>
using namespace std;
//赋值解析加载配置文件
    void MprpcConfig::LoadConfigFile(const char*config_file)
    {
        FILE*fp=fopen(config_file,"r");
        if(nullptr==fp)
        {
            cout<<config_file<<"is not exist"<<endl;
            exit(EXIT_FAILURE);
        }

        //读取配置文件，处理以下三种情况
        //1.注释   2.正确的配置项 =  3.去掉开头的空格
        while(!feof(fp))
        {
            //先处理前后的空格
            char buf[512]={0};
            fgets(buf,512,fp);//一行一行读
            //去掉字符串前面多余的空格
            string src_buf(buf);
            Trim(src_buf);
            //处理注释#
            if(src_buf[0]=='#'||src_buf.empty())
            {
                continue;
            }
            //处理正确的配置项
            int idx=src_buf.find('=');
            if(idx==-1)
            {
                continue;
            }
            string key;
            string value;
            key=src_buf.substr(0,idx);
            Trim(key);//去除前后空格
            //src_buf存放的结果为rpcserverip=127.0.0.1\n.
            int endidx=src_buf.find('\n',idx);//找\n位置，去掉最后一个\n
            value=src_buf.substr(idx+1,endidx-idx-1);
            Trim(value);
            m_configMap.insert({key,value});
        }
        fclose(fp);
    }
    //查询配置项信息
    string MprpcConfig::Load(const string&key)
    {
        auto it=m_configMap.find(key);//
        if(it==m_configMap.end())//没找到
        {
            return "";
        }
        return it->second;
    }

    //去掉字符串前后的空格
    void MprpcConfig::Trim(string&src_buf)
    {
        int idx=src_buf.find_first_not_of(' ');
            if(idx!=-1)
            {
                //说面字符串前面有空格
                src_buf=src_buf.substr(idx,src_buf.size()-idx);
            }
            //去掉后面的空格
            idx=src_buf.find_last_not_of(' ');
            if(idx!=-1)
            {
                //说明后面有空格
                src_buf=src_buf.substr(0,idx+1);
            }

    }