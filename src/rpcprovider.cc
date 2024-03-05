#include"rpcprovider.h"
#include<iostream>
#include<string>
#include"mprpcapplication.h"
#include"rpcheader.pb.h"
#include"logger.h"
using namespace std;
//把UserService对象发布到rpc节点上
void RpcProvider::NotifyService(google::protobuf::Service*service)//这里是基类指针
{

    ServiceInfo service_info;

    //获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor* pserviceDesc=service->GetDescriptor();
    //获取服务对象的名字
    string service_name=pserviceDesc->name();
    //获取服务对象service的方法的数量
    int methodCnt=pserviceDesc->method_count();
    LOG_INFO("service name:%s",service_name.c_str());
    service_info.m_service=service;
    for(int i=0;i<methodCnt;++i)
    {
        //获取了服务对象指定下标的服务方法的描述(抽象描述)
        const google::protobuf::MethodDescriptor*pmethodDesc=pserviceDesc->method(i);
        string method_name=pmethodDesc->name();
         service_info.m_methodMap.insert({method_name,pmethodDesc});
         LOG_INFO("method name:%s",method_name.c_str());
    }
    m_serviceInfoMap.insert({service_name,service_info});
}





void RpcProvider::Run()
{
    string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);
    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventloop,address,"RpcProvider");
    //绑定链接回调和消息读写回调方法
    server.setConnectionCallback(bind(&RpcProvider::OnConnection,this,placeholders::_1));

    server.setMessageCallback(bind(&RpcProvider::OnMessage,this,placeholders::_1,placeholders::_2,placeholders::_3));
    //设置muduo库的线程数量
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发布服务
    ZkClient zkCli;
    zkCli.Start();
    //Server_name为永久性节点，method_name为临时节点
    for(auto &sp:m_serviceInfoMap)
    {
        //service_path
        string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for(auto &mp:sp.second.m_methodMap)
        {
            //service_path/method_path
            string method_path=service_path+"/"+mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);//临时节点
        }
    }
    //启动网络服务
    server.start();
    m_eventloop.loop();
}
//新的socket链接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    //检测是否断开
    if(!conn->connected())
    {
        //rpc client的连接断开了
        LOG_ERR("rpc client disconnected");
        conn->shutdown();//将socket资源释放
    }
}
//已建立链接用户的读写事件回调
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr&conn,muduo::net::Buffer*buffer,muduo::Timestamp)
{
    //网络上接收的远程rpc调用请求的字符流，包括Login args
    string recv_buf=buffer->retrieveAllAsString();//转换成字符流放在recv_buf里。

    //从字符流中读取前4个字节的内容
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);

    //根据header_size读取数据头的原始字符流,反序列化数据得到rpc请求的详细信息
    string rpc_header_str=recv_buf.substr(4,header_size);
    string service_name;
    string method_name;
    uint32_t args_size;
    mprpc::RpcHeader rpcHeader;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功,
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    }
    else{
        //数据头反序列化失败,这里是日志
        return;
    }
    
    //获取rpc方法参数的字符流数据
    string args_str=recv_buf.substr(header_size+4,args_size);
//以上对rpc调用请求的字符流解析完成，下面开始进行调用方法
    
    //获取service对象和method方法
    auto it=m_serviceInfoMap.find(service_name);
    if(it==m_serviceInfoMap.end())//没找到
    {
        cout<<service_name<<"is not exist!"<<endl;
        return;
    }
    google::protobuf::Service*service=it->second.m_service;//获取service对象
    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end())
    {
        cout<<service_name<<":"<<method_name<<"is not exist"<<endl;
        return;
    }
    const google::protobuf::MethodDescriptor* method=mit->second;//获取method对象
    

    //生成rpc方法调用的请求resquest和响应response参数
    google::protobuf::Message*request=service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))//反序列化参数
    {
        cout<<"request parse error"<<endl;
    }
    google::protobuf::Message*response=service->GetResponsePrototype(method).New();//.New就是生成新对象，那么两个新对象会作为rpc方法的两个参数进行传入

    //给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure*done=google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,
    google::protobuf::Message*>(this,&RpcProvider::SendRpcResponse,conn,response);

    //在框架上根据远端rpc请求，调用当前rpc节点上发布方法
    service->CallMethod(method,nullptr,request,response,done);//相当于new userService().Login(controller,resquest,response,done).
    //最后一个参数done，让我们进行回调，我们可以把结果通过回调方式通过网络发送回去，
}

//在框架内部，RpcProvider和RpcConsumer协商好通信之间的protobuf类型
//service_name methon_name args
//最终格式为：
//16UserServiceLogin16zhang san123456
/*
header_size(4个字节)+header_str+args_str
16,也就是header_size,按二进制进行存储，所以字符流中前4个字节就是header_size
第二个16代表参数长度
*/

//Closure的回调操作，用于序列化rpc响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr&conn,google::protobuf::Message*response)
{
    string response_str;
    if(response->SerializeToString(&response_str))
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
    }
    else{
        cout<<"serialize response_str error"<<endl;
    }
    conn->shutdown();//模拟短连接
}