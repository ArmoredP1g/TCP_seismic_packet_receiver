#include <cstdint>
#include <vector>


struct __attribute__((packed)) PLASTD_time {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t millisecond;
    uint16_t microsecond;
    uint16_t nanosecond;
};

// 总39个字节
struct __attribute__((packed)) PacketHeader {
    uint32_t header = 0xE25C4876;   // (4byte)包头
    uint8_t system_id;              // (1byte)系统标识
    uint16_t source;                // (2byte)信源
    uint16_t destination;           // (2byte)信宿
    PLASTD_time time;               // (13byte)发送时刻
    uint16_t sequence_info;         // (2byte)序号信息
    uint16_t packet_length;         // (2byte)整包数据长度
    uint16_t group_count;           // (2byte)信息组数
    uint16_t device_type;           // (2byte)设备类型
    char device_id[5];              // (5byte)设备编号
    uint16_t msg_type_id;           // (2byte)信息类型编号
    uint16_t data_length;           // (2byte)每组数据段长度(字节)
};


// 总47+12*N个字节
struct SeismicData {
    uint8_t file_type;              // (1byte)文件类型
    PLASTD_time time;               // (13byte)时间
    double latitude;                // (8byte)纬度
    double longitude;               // (8byte)经度
    float altitude;                 // (4byte)海拔高度
    int32_t sampling_rate;          // (4byte)采样率
    float sensitivity;              // (4byte)灵敏度
    float conversion_factor;        // (4byte)转换因子       
    std::vector<int32_t> wave_x;    // (4*Nbyte)地震波形 X
    std::vector<int32_t> wave_y;    // (4*Nbyte)地震波形 Y
    std::vector<int32_t> wave_z;    // (4*Nbyte)地震波形 Z
    char checksum;                  // (1byte)校验位
};


// 总90+12*N个字节
struct Packet {
    PacketHeader header;                        
    SeismicData data;                // size:
    uint32_t tail = 0X8B8B7474;
};