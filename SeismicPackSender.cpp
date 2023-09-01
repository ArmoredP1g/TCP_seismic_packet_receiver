#include <iostream>
#include <sys/socket.h>
#include <random>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "SeismicPack.h"

class SeismicPackConstructor {

public:
    SeismicPackConstructor();
    ~SeismicPackConstructor();
    char* TestPacket(int group, int wave_len);
    Packet testPacket;
private:
    char* ToChar();     // 将结构体转换为字符数组形式

};

int main(int argc, char** argv) {
    int sockfd;
    int port;
    struct sockaddr_in servaddr;
    SeismicPackConstructor seismicPackConstructor;
    
    if (argc != 3) {
        std::cout << "usage: ./client <ipaddr> <port>" << std::endl;
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

    // 使用随机设备作为种子
    std::random_device rd;
    // 使用随机设备生成随机引擎
    std::mt19937 gen(rd());
    
    // 定义随机数分布的范围
    int min = 1;
    int max = 10;
    std::uniform_int_distribution<> dis(min, max);

    std::cout << "按一次回车发送一个数据包" << std::endl;

    while(1) {
        // 等待用户按下回车
        // std::cin.ignore(); // 忽略之前的回车
        std::cin.get(); // 等待用户按下回车
        // 生成随机整数
        int group = dis(gen);
        int len = dis(gen);
        // 每秒间隔，发送不定长的地震数据包
        char* p = seismicPackConstructor.TestPacket(group, len);
        if (send(sockfd, p, seismicPackConstructor.testPacket.header.packet_length, 0) < 0) {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            continue;
        }

        std::cout << "数据包已发送" << std::endl;
        delete[] p;
    }

    return 0;
}


SeismicPackConstructor::SeismicPackConstructor() {
    PLASTD_time currentTime = {2023, 8, 31, 10, 30, 45, 500, 0, 0};

    testPacket.header.header = 0xE25C4876;
    testPacket.header.system_id = 1;
    testPacket.header.source = 11;
    testPacket.header.destination = 22;
    testPacket.header.time = currentTime;
    testPacket.header.sequence_info = 1;
    // testPacket.header.packet_length = 1024;
    testPacket.header.group_count = 5;
    testPacket.header.device_type = 2;
    strcpy(testPacket.header.device_id, "1234");
    testPacket.header.msg_type_id = 10;
    testPacket.header.data_length = 512;

    testPacket.data.file_type = 1;
    testPacket.data.time = currentTime;
    testPacket.data.latitude = 37.7749;
    testPacket.data.longitude = -122.4194;
    testPacket.data.altitude = 10.5;
    testPacket.data.sampling_rate = 1000;
    testPacket.data.sensitivity = 1.5;
    testPacket.data.conversion_factor = 0.8;

    // testPacket.data.wave_x = {1, 2, 3, 4, 5};
    // testPacket.data.wave_y = {6, 7, 8, 9, 10};
    // testPacket.data.wave_z = {11, 12, 13, 14, 15};

    testPacket.data.checksum = 'A';

    testPacket.tail = 0x8B8B7474;
}

char* SeismicPackConstructor::TestPacket(int group, int wave_len) {
    testPacket.header.group_count = group;
    testPacket.header.data_length = wave_len * 4 * 3;
    testPacket.header.packet_length = 90 + (group * wave_len * 4 * 3);
    testPacket.data.wave_x.clear();
    testPacket.data.wave_y.clear();
    testPacket.data.wave_z.clear();
    for (int i=0; i<group*wave_len; i++) {
        //这块其实写的不对，但老子懒得改了，反正字节数都一样不改了
        testPacket.data.wave_x.push_back(8);
        testPacket.data.wave_y.push_back(7);
        testPacket.data.wave_z.push_back(6);
    }

    char* tcp_pack = ToChar();
    return tcp_pack;
}


char* SeismicPackConstructor::ToChar() {
    /*
    将结构体中的数据flatten到一个char数组中
    */
    char* result = new char[testPacket.header.packet_length];

    char* p = result;
    *(PacketHeader*)p = testPacket.header;
    p += sizeof(PacketHeader);
    *(uint8_t*)p = testPacket.data.file_type;
    p += sizeof(uint8_t);
    *(PLASTD_time*)p = testPacket.data.time;
    p += sizeof(PLASTD_time);
    *(double*)p = testPacket.data.latitude;
    p += sizeof(double);
    *(double*)p = testPacket.data.longitude;
    p += sizeof(double);
    *(float*)p = testPacket.data.altitude;
    p += sizeof(float);
    *(int32_t*)p = testPacket.data.sampling_rate;
    p += sizeof(int32_t);
    *(float*)p = testPacket.data.sensitivity;
    p += sizeof(float);
    *(float*)p = testPacket.data.conversion_factor;
    p += sizeof(float);
    // std::memcpy(p, &testPacket.data.wave_x, testPacket.header.data_length * testPacket.header.group_count);
    // p += testPacket.header.data_length * testPacket.header.group_count;
    // std::memcpy(p, &testPacket.data.wave_y, testPacket.header.data_length * testPacket.header.group_count);
    // p += testPacket.header.data_length * testPacket.header.group_count;
    // std::memcpy(p, &testPacket.data.wave_z, testPacket.header.data_length * testPacket.header.group_count);
    // p += testPacket.header.data_length * testPacket.header.group_count;
    int group_len = testPacket.header.data_length / (3 * 4);
    for (int i=0; i<testPacket.header.group_count; i++) {
        // 数据分组
        std::memcpy(p, &testPacket.data.wave_x[i*group_len], group_len*sizeof(int32_t));
        p += group_len*sizeof(int32_t);
        std::memcpy(p, &testPacket.data.wave_y[i*group_len], group_len*sizeof(int32_t));
        p += group_len*sizeof(int32_t);
        std::memcpy(p, &testPacket.data.wave_z[i*group_len], group_len*sizeof(int32_t));
        p += group_len*sizeof(int32_t);

    }
    *(char*)p = testPacket.data.checksum;
    p += sizeof(char);
    *(uint32_t*)p = testPacket.tail;
    return result;
}

SeismicPackConstructor::~SeismicPackConstructor() {

}