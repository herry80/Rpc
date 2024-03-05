#include"test.pb.h"
#include<iostream>
#include<string>
//using namespace std;
using namespace fixbug;

int main()
{
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");
    std::string str;
    if(req.SerializeToString(&str))
    {
        std::cout<<str.c_str()<<std::endl;
    }
    LoginRequest reqB;
    if(reqB.ParseFromString(str))
    {
        std::cout<<reqB.name()<<std::endl;
        std::cout<<reqB.pwd()<<std::endl;
    }
    return 0;
}