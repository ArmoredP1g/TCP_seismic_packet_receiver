# TCP_seismic_packet_receiver示例说明

## 文件说明

你大概只需要：

SeismicPackReceiver.cpp （数据包解析）

SeismicPack.h		（数据包定义）

这俩文件，



有一个SeismicPackSender是用来模拟发送数据包测试的，启动命令是 ./SeismicPackSender ip地址 端口号，按一下回车发一个包

应该不需要任何额外依赖



编译命令配置是task.json里的"地震数据包接收解析"，里边的路径你自己改一下



貌似还有个记录历史台站的功能你得实现，可以存个文件啥的，或者干脆也存redis里得了。