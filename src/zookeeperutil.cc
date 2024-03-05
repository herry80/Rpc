#include"include/zookeeperutil.h"
#include"include/mprpcapplication.h"
#include<semaphore.h>
#include<iostream>
using namespace std;

void global_watcher(zhandle_t*zh,int type,int state,const char*path,void*watcherCtx)
{
    if(type==ZOO_SESSION_EVENT)//回调的消息类型是和会话相关的消息类型
    {
        if(state==ZOO_CONNECTED_STATE)//连接成功了
        {
            sem_t*sem=(sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient():m_zhandle(nullptr)
{}
ZkClient::~ZkClient()
{
    if(m_zhandle!=nullptr)//连接zookeeper server成功
    {
        zookeeper_close(m_zhandle);//关闭句柄，释放资源
    }
}
void ZkClient::Start()
{
    string host=MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    string port=MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    string connstr=host+":"+port;

    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);//创建句柄成功，还没连接成功
    if(nullptr==m_zhandle)
    {
        cout<<"zookeeper_init error!"<<endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;//创建一个信号量
    sem_init(&sem,0,0);
    zoo_set_context(m_zhandle,&sem);

    sem_wait(&sem);
    cout<<"zookeeper_init success!"<<endl;
}
void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen=sizeof(path_buffer);
    int flag;
    flag=zoo_exists(m_zhandle,path,0,nullptr);//先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
    if(ZNONODE==flag)
    {
        flag=zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag==ZOK)
        {
            cout<<"znode create success...path:"<<path<<endl;
        }
        else{
            cout<<"flag:"<<flag<<endl;
            cout<<"znode create error...path:"<<path<<endl;
            LOG_ERR("znode create error...path:%s",path);
            exit(EXIT_FAILURE);
        }
    }
}
//根据指定的path，获取相应的节点值
string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen=sizeof(buffer);
    int flag=zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);
    if(flag!=ZOK)
    {
        cout<<"get znode error...path:"<<path<<endl;
        LOG_ERR("get znode error...path:%s",path);
        return "";
    }
    else{
        return buffer;
    }
}