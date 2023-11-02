#ifndef __APP_UPDATE_H__
#define __APP_UPDATE_H__

#warning 选择App应写入的位置
#define AppAddr 0x08002800

typedef struct {
    unsigned int AppSize;               // App全包大小
    unsigned short AppCrc;              // App全包校验码
    unsigned int SignalPackSize;        // 单包大小（包含bin+信息）
    unsigned short TotalNumber;         // 包的总数（切分成多少份）
    unsigned char BinFrameInformation;  // Bin帧的信息长度（与App无关的内容）
} AppMessage;

typedef struct {
    unsigned short DataID;   // 数据的编号
    unsigned int DataSize;   // 当前收到数据段的大小
    unsigned short DataCrc;  // 当前收到数据尾部的校验值
} DataGetMessage;

typedef struct {
    unsigned char ErrorCount;       // 错误计数
    unsigned char UpdateStartFlag;  // 升级开始标志
    unsigned char UpdateEndFlag;    // 升级结束标志
    unsigned char UpdateResult;     // 接收完成后对写入flash的App进行全包校验，1正确0错误
} AppUpdateFlag;

void UARTDataDeal(unsigned char *data, unsigned short datalen);
void UpdateStart(unsigned char *data);
void GetBinData(unsigned char *data);
void UpdateEnd(unsigned char *data);
void GetCommand(unsigned char *data);
void Reply(unsigned char *data, unsigned char len);
void WriteApptoFlash(unsigned char *data);
#endif
