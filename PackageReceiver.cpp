#include<stdio.h>
#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

#define MAXLINE 4096

int main(int argc, char** argv) {
    int listenfd, connfd;                        // 监听套接字和连接套接字
    struct sockaddr_in servaddr;                 // 服务器地址结构体
    char buff[4096];                             // 缓冲区
    int port;                                    // 端口号
    int n;                                       // 接收到的字节数

    if (argc < 2) {
        printf("usage: ./client <port>\n");
        return 0;
    }

    // 获取端口参数
    try {
        port = std::stoi(argv[1]);
    }
    catch (const std::invalid_argument& e) {
        std::cout << "check your port" << std::endl;
    }


    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));      // 清空服务器地址结构体
    servaddr.sin_family = AF_INET;                // IPv4 协议
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意本地地址
    servaddr.sin_port = htons(port);              // 设置端口号为 6666

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("======waiting for client's request======\n");

    if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1) {
        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    while ((n = recv(connfd, buff, sizeof(buff) - 1, 0)) > 0) {
        buff[n] = '\0';                            // 在接收到的数据末尾添加字符串结束符
        printf("recv msg from client: %s\n", buff); // 打印接收到的数据
    }

    if (n < 0) {
        printf("recv socket error: %s(errno: %d)\n", strerror(errno), errno);
    }

    close(connfd); // 关闭连接套接字
    close(listenfd); // 关闭监听套接字
    return 0;
}