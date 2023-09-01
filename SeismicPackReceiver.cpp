#include <stdio.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "SeismicPack.h"

Packet SeismicPackParser(char* pack);

int main(int argc, char** argv) {
    int listenfd, connfd=-1;                        // 监听套接字和连接套接字
    struct sockaddr_in servaddr;                 // 服务器地址结构体
    char buff[4096*4];                             // 缓冲区
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



    while (1) {
        if ((n = recv(connfd, buff, sizeof(buff) - 1, 0)) <= 0 || connfd==-1) {
            printf("======waiting for client's request======\n");

            if ((connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1) {
                printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
                return 0;
            }
            continue;
        }

        buff[n] = '\0';                            // 在接收到的数据末尾添加字符串结束符
        
        /*解析数据包到结构体中*/
        char* p = buff;
        Packet pack = SeismicPackParser(p);

        // 打印数据包信息
        std::cout << "数据包长度：\t" << pack.header.packet_length << std::endl;
        std::cout << "数据组数：\t" << pack.header.group_count << std::endl;
        std::cout << "时间：\t" << pack.header.time.year << std::endl;
        std::cout << "包头：\t" << std::endl;
        printf("0x%08x\n", pack.header.header);
        std::cout << "包尾：\t" << std::endl;
        printf("0x%08x\n", pack.tail);
    }

    if (n < 0) {
        printf("recv socket error: %s(errno: %d)\n", strerror(errno), errno);
    }

    close(connfd); // 关闭连接套接字
    close(listenfd); // 关闭监听套接字

    return 0;
}


Packet SeismicPackParser(char* pack) {
    Packet result;
    std::memcpy(&result.header, pack, sizeof(PacketHeader));
    pack += sizeof(PacketHeader);
    std::memcpy(&result.data.file_type, pack, sizeof(uint8_t));
    pack += sizeof(uint8_t);
    std::memcpy(&result.data.time, pack, sizeof(PLASTD_time));
    pack += sizeof(PLASTD_time);
    std::memcpy(&result.data.latitude, pack, sizeof(double));
    pack += sizeof(double);
    std::memcpy(&result.data.longitude, pack, sizeof(double));
    pack += sizeof(double);
    std::memcpy(&result.data.altitude, pack, sizeof(float));
    pack += sizeof(float);
    std::memcpy(&result.data.sampling_rate, pack, sizeof(int32_t));
    pack += sizeof(int32_t);
    std::memcpy(&result.data.sensitivity, pack, sizeof(float));
    pack += sizeof(float);
    std::memcpy(&result.data.conversion_factor, pack, sizeof(float));
    pack += sizeof(float);
    int group_len = result.header.data_length / (3 * 4);

    result.data.wave_x = std::vector<int32_t>(group_len * result.header.group_count);
    result.data.wave_y = std::vector<int32_t>(group_len * result.header.group_count);
    result.data.wave_z = std::vector<int32_t>(group_len * result.header.group_count);

    for (int i=0; i<result.header.group_count; i++) {
        std::memcpy(&result.data.wave_x[i*group_len], pack, group_len*sizeof(int32_t));
        pack += group_len*sizeof(int32_t);
        std::memcpy(&result.data.wave_y[i*group_len], pack, group_len*sizeof(int32_t));
        pack += group_len*sizeof(int32_t);
        std::memcpy(&result.data.wave_z[i*group_len], pack, group_len*sizeof(int32_t));
        pack += group_len*sizeof(int32_t);
    }
    std::memcpy(&result.data.checksum, pack, sizeof(char));
    pack += sizeof(char);
    std::memcpy(&result.tail, pack, sizeof(uint32_t));
    
    return result;
}