#pragma once
#include<semaphore.h>//信号量
#include<zookeeper/zookeeper.h>
#include<string>
#include"logger.h"
using namespace std;
//封装的zk客户端类
class ZkClient
{
    public:
    ZkClient();
    ~ZkClient();
    //zkclient启动连接zkserver
    void Start();
    //在zkserver上根据指定的path创建znode节点
    void Create(const char*path,const char*data,int datalen,int state=0);//state决定是永久性节点还是临时性节点
    //根据参数指定的znode节点路劲，获得znode节点的值
    string GetData(const char*path);
    private:
    //zk的客户端句柄
    zhandle_t*m_zhandle;
};