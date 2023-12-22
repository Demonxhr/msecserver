#include "secserver.hpp"
#include <functional>
#include <string>
#include "json.hpp"
#include "secservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
// 初始化聊天服务器对象
SecServer::SecServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&SecServer::onConnection, this, _1));

    // 注册消息回调
    _server.setMessageCallback(std::bind(&SecServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

// 启动服务
void SecServer::start()
{
    _server.start();
}

// 上报链接相关信息的回调函数
void SecServer::onConnection(const TcpConnectionPtr &conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        SecService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void SecServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //数据反序列化
    json js = json::parse(buf);
    //达到的目的：完全解耦网络模块的代码和业务模块的代码
    //通过js["msgid"] 获取 业务handler ==》 conn js time
    auto msgHandler = SecService::instance()->getHandler(js["msgid"].get<int>());
    //回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn,js,time);
}
