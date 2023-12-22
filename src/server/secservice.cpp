#include "secservice.hpp"
#include "public.hpp"
#include "rsacrypto.hpp"
#include "hash.hpp"
#include "aescrypto.hpp"
#include "base64.hpp"
#include <muduo/base/Logging.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
using namespace muduo;

enum KeyLen {Len16=16,Len24=24,Len32=32};
//生成随机数
string getRandKey(KeyLen len)
{
	//设置随机种子 如果不设置 第一次都是一样的
	srand(time(NULL));
	int flag = 0;
	string randStr = string();
	char *cs = "~!@#$%^&*()_+{}|\';";
	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;
		switch (flag)
		{
		case 0:    //a-z
			randStr.append(1, 'a' + rand() % 26);
			break;
		case 1:    //A-Z
			randStr.append(1, 'A' + rand() % 26);
			break;
		case 2:    //0-9
			randStr.append(1, '0' + rand() % 10);
			break;
		case 3:    //特殊字符
			randStr.append(1, cs[rand() % strlen(cs)]);
			break;
		default:
			break;

		}
	}
	return randStr;
}

SecService *SecService::instance()
{
    static SecService service;
    return &service;
}



SecService::SecService()
{
    _msgHandlerMap.insert({DISPLAY_MSG, std::bind(&SecService::backdisplay, this, _1, _2, _3)});
    _msgHandlerMap.insert({SEC_AGREE_MSG,std::bind(&SecService::secKeyAgree, this, _1, _2, _3)});
}

// 回显事件
void SecService::backdisplay(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string data = js["data"];
    int id = js["id"].get<int>();

    string key = _seckeyMap[id];
    Base64 base64;
    AesCrypto aes(key);

    string msg = "hello world";

    string data1 = base64.Base64Decode(data);
    string data2 = aes.aesCBCDecrypt(data1);
    string data3 = aes.aesCBCEncrypt(data2);
    string data5 = aes.aesCBCEncrypt(msg);
    string data4 = base64.Base64Encode(data3);
    cout << "收到的数据：" << data <<endl;
    cout << "base64解码后的数据： " << data1 << endl;
    cout << "aes解密后的数据，即还原后的数据： " << data2 << endl;
    cout << "aes重新加密的数据： " << data3 << endl;
    cout << "要发送的数据，即base64编码后的数据：" << data4 << endl;
    cout << "data5: " << data5 << endl;
    cout << "aes密钥key：" << key << endl;



	stringstream ss;   
	for (int i = 0; i < key.length(); i++)
	{
		int val = (int)key[i];   
		ss << "0x"<<hex << val<<" ";   
	}
 
	string hexStr = ss.str();   
 
	cout <<"16进制：" << hexStr << endl;


    js["data"] = data4;
    


    // Base64 base64;
    // string ddata = base64.Base64Decode(data);

    // cout << "base64加密数据： " << data <<endl;
    // data = ddata;
    
    // cout << "base64解密后数据： " << data <<endl;
    // string key = _seckeyMap[id];
    // cout << "对称加密密钥： " << key <<endl; 
    // AesCrypto aes(key);
    // string msg = aes.aesCBCDecrypt(data);
    // cout << msg << endl;
    // cout << msg.length() << endl;
    // data = aes.aesCBCEncrypt(msg);
    // cout << "aes加密后msg: " << data <<  endl;
    // string send_data = base64.Base64Encode(data);
    // cout << "send_data： " << send_data << endl;
    // js["data"] = send_data;
    conn->send(js.dump());
}

// 密钥协商
void SecService::secKeyAgree(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string data = js["data"];
    string sign = js["sign"];
    int id = js["id"].get<int>();

    ofstream ofs("/home/aa/chat/secsever/bin/skey/public.pem");
    ofs << data;
    ofs.close();

    RsaCrypto rsa("/home/aa/chat/secsever/bin/skey/public.pem",false);
    Hash mHash(T_SHA1);
    mHash.addData(data);

    json reJs;
    reJs["msgid"] = SEC_AGREE_MSG;
    bool signVerify = rsa.rsaVerify(mHash.result(),sign);
    if(!signVerify)
    {
        cout << "签名校验失败" << endl;
        reJs["status"] = false;
    }
    else
    {
        cout << "签名校验成功" << endl;
        reJs["status"] = true;
        //生成对称加密密钥
        string key = getRandKey(Len16);
        cout << "对称加密密钥: " << key << endl;
        //保存到数据库
        if(_seckeyMap.find(id)==_seckeyMap.end())
        {
            _seckeyMap.insert({id,key});
        }
        else
        {
            _seckeyMap[id] = key;
        }
        

        //用公钥加密对称加密密钥
        string secKey = rsa.rsaPubKeyEncrypt(key);
        reJs["data"] = secKey;


    }
    conn->send(reJs.dump());
}

// 处理客户端异常退出
void SecService::clientCloseException(const TcpConnectionPtr &conn)
{

}



// 获取消息对应的处理器
MsgHandler SecService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}