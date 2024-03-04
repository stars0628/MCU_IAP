# 使用事项
1. 注意根据warning完善AppUpdate程序，例如补充串口发送函数，补充flash写入函数等
2. 接收完串口数据后调用UpdateStart程序即可
3. 所有bin数据接收到并且写入flash并校验成功会将appdateflag.UpdateResult置1，可以判断此标志来执行跳转

# 数据接收发送格式
## 上位机发送下位机接收
1. 传输起始帧：包含AppMessage的所有内容：
格式："55aa" + 4字节AppSize + 2字节AppCrc + 4字节SignalPackSize + 2字节TotalNumber + 1字节每个数据帧包含的额外信息长度(除了bin数据外的所有数据) + crc16-modbus
2. Bin数据帧：发送Bin的编号和Bin数据：
格式："66bb" + 2字节编号+Bin段数据 + crc16-modbus
3. 传输结束帧：所有bin文件传输完成发送：
格式："77cc" + crc16-modbus
4. 其他命令帧：00重置升级：
格式："88dd" + 1个字节命令 + crc16-modbus

## 上位机接收下位机发送
1. 有如下类型：
0x00起始帧 0x01bin数据帧 0x02结束帧 0x03命令帧
2. 接收成功：
格式："11ee" + 1字节类型 + crc16-modbus
3. 校验错误：
格式；"22ff" 0xff + crc16-modbus
4. 数据异常：
格式："3344" + 1字节类型 + crc16-modbus
