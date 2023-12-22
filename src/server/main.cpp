#include <iostream>
#include <signal.h>
#include "secserver.hpp"
#include "secservice.hpp"
using namespace std;

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    SecServer server(&loop, addr, "SecServer");

    server.start();
    loop.loop();

    return 0;
}