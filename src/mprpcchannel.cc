#include"include/mprpcchannel.h"
#include"rpcheader.pb.h"
#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include"mprpcapplication.h"
#include<unistd.h>
#include"mprpccontroller.h"
#include"zookeeperutil.h"
using namespace std;

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor*sd=method->service();
    string service_name=sd->name();
    string method_name=method->name();
    
    //获取参数的序列化字符串长度 
    string args_str;
    uint32_t args_size=0;
    if(request->SerializeToString(&args_str))
    {
        args_size=args_str.size();
    }
    else{
        //cout<<"Serialize request error"<<endl;
        controller->SetFailed("Serialize request error");
        return;
    }
    //定义rpc 的请求header
    mprpc::RpcHeader header;
    header.set_service_name(service_name);
    header.set_method_name(method_name);
    header.set_args_size(args_size);

    string rpc_header_str;
    uint32_t header_size=0;
    if(header.SerializeToString(&rpc_header_str))
    {
        header_size=rpc_header_str.size();
    }
    else{
        //cout<<"Serialize header error"<<endl;
        controller->SetFailed("Serialize header error");
        return;
    }
    //组织带发送的rpc请求的字符串
    string send_rpc_str;
    send_rpc_str.insert(0,string((char*)&header_size,4));//header_size
    send_rpc_str+=rpc_header_str;//header
    send_rpc_str+=args_str;//args
    //使用tcp编程完成rpc方法的远程调用
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1)
    {
        //cout<<"creat socket error!:"<<errno<<endl;
        char errtxt[512]={0};
        sprintf(errtxt,"creat socket error!errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }
    //获取ip地址和port
    //string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    //uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    ZkClient zkCli;
    zkCli.Start();
    string method_path="/"+service_name+"/"+method_name;
    string host_data=zkCli.GetData(method_path.c_str());
    if(host_data=="")
    {
        controller->SetFailed(method_path+"is not exist!");
        return;
    }
    int idx=host_data.find(":");
    if(idx==-1)
    {
        controller->SetFailed(method_path+"address is invalid!");
        return;
    }
    string ip=host_data.substr(0,idx);
    uint16_t port=atoi(host_data.substr(idx+1,host_data.size()-idx).c_str());

    
    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(port);
    saddr.sin_addr.s_addr=inet_addr(ip.c_str());

    int res=connect(clientfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(res==-1)
    {
        //cout<<"connect failed:"<<errno<<endl;
        char errtxt[512]={0};
        sprintf(errtxt,"connect failed!errno:%d",errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    if(-1==send(clientfd,send_rpc_str.c_str(),send_rpc_str.size(),0))
    {
        //cout<<"send failed:"<<errno<<endl;
        char errtxt[512]={0};
        sprintf(errtxt,"send failed!errno:%d",errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        //exit(EXIT_FAILURE);
        return;
    }
    //接收rpc的响应值
    char buf[1024]={0};
    int recv_size=0;
    if(-1==(recv_size=recv(clientfd,buf,1024,0)))
    {
        //cout<<"recv failed:"<<errno<<endl;
        char errtxt[512]={0};
        sprintf(errtxt,"recv failed!errno:%d",errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
  
    //写入response
    string response_str(buf,recv_size);
    if(!response->ParseFromString(response_str))
    {
        //cout<<"parse error:"<<endl;
        //char errtxt[512]={0};
        //sprintf(errtxt,"parse error!errno:%s",buf);
        controller->SetFailed("parse error!");
        close(clientfd);
        return;
    }
    close(clientfd);
}