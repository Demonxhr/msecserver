#include "rsacrypto.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
int test_rsa()
{
    // 生成密钥对
    RsaCrypto rsa;
    rsa.generateRsakey(1024);
    // 读公钥文件
	ifstream ifs("public.pem");
	stringstream str;
	str << ifs.rdbuf();
    cout << "公钥：";
    cout << str.str() << endl;

    // 读入私钥文件
    ifstream ifs1("private.pem");
    stringstream str1;
	str << ifs1.rdbuf();
    cout << "私钥：";
    cout << str1.str() << endl;

    // 要加密的数据
    string data = "hello world";
    cout << "原始数据：" << data << endl;
    //公钥加密
    data = rsa.rsaPubKeyEncrypt(data);
    cout << "公钥加密后数据：" << data << endl;
    //私钥解密
    data = rsa.rsaPriKeyDecrypt(data);
    cout << "私钥解密后数据：" << data << endl;
    //私钥签名
    string sdata = rsa.rsaSign(data);
    cout << "私钥签名后的签名数据：" << sdata << endl;
    //公钥验签
    bool flag = rsa.rsaVerify(data,sdata);
    cout << "flag:" << flag << endl;



    return 0;
}