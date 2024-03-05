#pragma once
#include"google/protobuf/service.h"
//框架提供的专门负责发布rpc服务的网络对象类
#include<memory>
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<string>
#include<functional>
#include<google/protobuf/descriptor.h>
#include<unordered_map>
#include"zookeeperutil.h"
using namespace std;
class RpcProvider
{
    public:
    //这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service*service);//这里是基类指针
    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

    private:

    struct ServiceInfo//service服务类型信息
    {
        google::protobuf::Service*m_service;//保存服务对象
        unordered_map<string,const google::protobuf::MethodDescriptor*>m_methodMap;//保存服务方法
    };
    //相当于一张表，存储注册成功的服务对象和其服务方法的所有信息
    unordered_map<string,ServiceInfo> m_serviceInfoMap;

    //组合了Tcpserver
    //unique_ptr<muduo::net::TcpServer> m_tcpserverPtr;
    //组合了EventLoop
    muduo::net::EventLoop m_eventloop;
    //新的socket链接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //已建立链接用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr&,
                            muduo::net::Buffer*,
                            muduo::Timestamp);
    //Closure的回调操作，用于序列化rpc响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message*);
};