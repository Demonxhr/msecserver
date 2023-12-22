#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

//表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp)>;

class SecService
{
public:
    // 获取单例对象的接口函数
    static SecService* instance();

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    // 回显事件
    void backdisplay(const TcpConnectionPtr &conn,json &js,Timestamp time);

    // 密钥协商
    void secKeyAgree(const TcpConnectionPtr &conn,json &js,Timestamp time);
private:
    SecService();

    // 存储消息id和其对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;

    // 存储id对应的密钥
    unordered_map<int,string> _seckeyMap;

};