#include<iostream>
#include"mprpcapplication.h"
#include"../user.pb.h"
using namespace std;

int main(int argc,char**argv)
{
    MprpcApplication::Init(argc,argv);//一定要先调用框架的初始化函数

    //演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub sub(new MprpcChannel());//创建一个UserserviceRpc_Stub的对象，需要传一个RpcChannel指针,所以需要创建
    //一个MprpcChannel对象继承RpcChannel，重写里面的callMethond方法

    //rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    //rpc方法的响应
    fixbug::LoginResponse response;

    //定义一个controller对象
    MprpcController controller;
    //发起rpc方法的调用 同步的rpc调用过程 底层就是调用MprpcChannel::callMethond的方法
    sub.Login(&controller,&request,&response,nullptr);
    //通过controller对象判断过程中是否出现问题
    if(controller.Failed())
    {
        cout<<controller.ErrorText()<<endl;
    }
    else{
        // 一次rpc调用结果完成，读response响应结果
        if (response.result().errcode() == 0)
        {
            cout << "rpc login response:" << response.sucess() << endl;
        }
        else
        {
            cout << "rpc login response error" << response.result().errmsg() << endl;
        }
    }
    


    //演示调用远程发布的rpc方法Register
    //fixbug::UserServiceRpc_Stub sub1(new MprpcChannel());不需要写了
    fixbug::RegisterRequest registerrequest;
    registerrequest.set_id(0716);
    registerrequest.set_name("li si");
    registerrequest.set_pwd("987654");

    fixbug::RegisterResponse registerresponse;
    controller.Reset();
    sub.Resist(&controller,&registerrequest,&registerresponse,nullptr);
    if(controller.Failed())
    {
        cout<<controller.ErrorText()<<endl;
    }
    else{
        // 一次rpc调用结果完成，读response响应结果
        if (registerresponse.result().errcode() == 0)
        {
            cout << "rpc register response:" << registerresponse.sucess() << endl;
        }
        else
        {
            cout << "rpc register response error" << registerresponse.result().errmsg() << endl;
        }
    }
    

    return 0;
}