#include<stdio.h>
#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#define MAXLINE 4096

int main(int argc, char** argv) {
    int sockfd, n;
    int port;
    int maxSendSize = 4096;
    struct sockaddr_in servaddr;

    if (argc < 3) {
        printf("usage: ./client <ipaddress> <port> <maxSendSize>(默认4096)\n");
        return 0;
    }

    // 将 IP 地址转换为网络字节序并设置到服务器地址结构体中
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("inet_pton error for %s\n", argv[1]);
        return 0;
    }

    // 获取端口参数
    try {
        port = std::stoi(argv[2]);
    }
    catch (const std::invalid_argument& e) {
        std::cout << "check your port" << std::endl;
    }

    // 检查是否提供了最大发送字数参数
    if (argc >= 4) {
        maxSendSize = std::stoi(argv[3]);
    }

    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);


    // 建立与服务器的连接
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("send msg to server: \n");
    // 从标准输入循环读取用户输入的消息
    // fgets(sendline, 4096, stdin);

    for (std::string sendline; std::getline(std::cin, sendline);) {
        if (sendline.length() > 4096) {
            sendline.resize(4096);
        }

        // 在这里使用 sendline 进行后续操作
        // 将用户输入的消息发送给服务器
        if (send(sockfd, sendline.c_str(), sendline.size(), 0) < 0) {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            continue;
        }

        // 接收服务器的响应,不接收响应会出现Broken pipe异常
        // char recvline[MAXLINE + 1];
        // memset(recvline, 0, sizeof(recvline));
        // if ((n = recv(sockfd, recvline, MAXLINE, 0)) < 0) {
        //     printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
        //     continue;
        // }

        // 处理服务器的响应
        // printf("Received from server: %s\n", recvline);

        sendline.clear();
    }
    

    // 关闭套接字
    close(sockfd);
    return 0;
}