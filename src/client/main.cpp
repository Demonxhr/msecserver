#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <thread>
#include <unistd.h>
#include <semaphore.h>
#include <atomic>

#include "json.hpp"
#include "public.hpp"
#include "rsacrypto.hpp"
#include "hash.hpp"
#include "aescrypto.hpp"
#include "base64.hpp"
using json = nlohmann::json;
using namespace std;

// 用于读写线程之间的通信
sem_t rwsem;
// 接收线程
void readTaskHandler(int clientfd);
// 密钥协商处理函数
void secKeyAgree(json js);
// 密钥协商是否成功
atomic_bool secKeyIsSuccess{false};
//用于保存对称加密密钥
string secKey = "";

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建client端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

        // 填写client需要连接的server信息ip+port
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // client和server进行连接
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }
    // 初始化读写线程通信用的信号量
    sem_init(&rwsem, 0, 0);

        // 连接服务器成功，启动接收子线程
    std::thread readTask(readTaskHandler, clientfd); // pthread_create
    readTask.detach();                               // pthread_detach


    // main线程用于接收用户输入，负责发送数据
    for (;;)
    {
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. seckeyagree" << endl;
        cout << "2. seckeycheck" << endl;
        cout << "3. seckeyoff" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读掉缓冲区残留的回车
         switch (choice)
        {
        case 1:
        {
            RsaCrypto rsa;
            rsa.generateRsakey(1024,"/home/aa/chat/secsever/bin/ckey/public.pem","/home/aa/chat/secsever/bin/ckey/private.pem");

            //读公钥文件
            ifstream ifs("/home/aa/chat/secsever/bin/ckey/public.pem");
            stringstream mStr;
            mStr << ifs.rdbuf();

            //将公钥hash
            Hash mHash(T_SHA1);
            //结果保存在mHash.result()中
            mHash.addData(mStr.str());


            //序列化
            json js;
            js["msgid"] = SEC_AGREE_MSG;
            js["id"] = 23;
            js["data"] = mStr.str();
            //对hash后的公钥签名
            js["sign"] = rsa.rsaSign(mHash.result());

            string sendMsg = js.dump();

            secKeyIsSuccess = false;
            //发送数据
            int len = send(clientfd,sendMsg.c_str(),strlen(sendMsg.c_str())+1,0);
            if (len == -1)
            {
                cerr << "send login msg error:" << sendMsg << endl;
            }

            // 等待信号量，由子线程处理完登录的响应消息后，通知这里
            sem_wait(&rwsem);
            if(secKeyIsSuccess)
            {
                string msg = "hello world";
                Base64 base64;
                AesCrypto aes(secKey);
                json js;

                string data = aes.aesCBCEncrypt(msg);
                string data1 =base64.Base64Encode(data);
                cout << "原始字符串：" << msg <<endl;
                cout << "aes加密后字符串：" << data << endl;
                cout << "base64加密后字符串：" << data1 << endl;
                cout << "对称加密密钥：" << secKey <<endl;
                stringstream ss;   
                for (int i = 0; i < secKey.length(); i++)
                {
                    int val = (int)secKey[i];   
                    ss << "0x"<<hex << val<<" ";   
                }
            
                string hexStr = ss.str();   
            
                cout <<"16进制：" << hexStr << endl;

                js["msgid"] = DISPLAY_MSG;
                js["data"] = data1;
                js["id"] = 23;
                
                sendMsg = js.dump();
                len = send(clientfd,sendMsg.c_str(),strlen(sendMsg.c_str())+1,0);
                if (len == -1)
                {
                    cerr << "send login msg error:" << sendMsg << endl;
                }
                sem_wait(&rwsem);

            }
            break;

        }
        case 2:
            break;
        case 3:
            break;
        default:
            cerr << "invalid input!" << endl;
            break;
        }
    }
    return 0;
}


// 子线程 - 接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);  // 阻塞了
        if (-1 == len || 0 == len)
        {
            close(clientfd);
            exit(-1);
        }

        // 接收ChatServer转发的数据，反序列化生成json数据对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if(SEC_AGREE_MSG == msgtype)
        {
            secKeyAgree(js);
            sem_post(&rwsem);
            continue;
        }
        if(SEC_CHECK_MSG == msgtype)
        {

        }
        if(SEC_OFF_MSG == msgtype)
        {

        }
        if(DISPLAY_MSG == msgtype)
        {
            AesCrypto aes(secKey);
            Base64 base64;
            string data = js["data"];
            string data1 = base64.Base64Decode(data);
            string data2 = aes.aesCBCDecrypt(data1); 
            cout << "收到回显shuju: " << data << endl;
            cout << "base64解码后数据： "<< data1 << endl;
            cout << "aes解密后的数据：" << data2 <<endl;
            sem_post(&rwsem);
            continue;
        }
    }
}

void secKeyAgree(json js)
{
    string data = js["data"];
    bool status = js["status"];
    if(!status)
    {
        secKeyIsSuccess = false;
    } 
    else
    {
        secKeyIsSuccess = true;
        RsaCrypto rsa("./ckey/private.pem");
        secKey = rsa.rsaPriKeyDecrypt(data);
        cout << "对称加密密钥：" << secKey << endl; 
    }
}
