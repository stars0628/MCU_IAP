#include "datadeal.h"
#include <QDebug>

DataDeal::DataDeal()
{

}

// 计算CRC16 Modbus校验码
unsigned short DataDeal::crc16_modbus(unsigned char *data, unsigned int len)
{
    unsigned short crc = 0xFFFF;
    for (unsigned int i = 0; i < len; i++) {
        crc ^= (unsigned short)data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void DataDeal::HeadFrameData(unsigned int AppSize,unsigned short Crc,unsigned int SingalBinSize,unsigned short Number,unsigned char BinFrameOtherLength,unsigned char *headdata)
{
    unsigned char AppSizebyte1 = (AppSize >> 24) & 0xFF; // 获取高位字节
    unsigned char AppSizebyte2 = (AppSize >> 16) & 0xFF; // 获取次高位字节
    unsigned char AppSizebyte3 = (AppSize >> 8) & 0xFF;  // 获取次低位字节
    unsigned char AppSizebyte4 = AppSize & 0xFF;         // 获取低位字节

    unsigned char Crcbyte1 = (Crc >> 8) & 0xFF;  // 获取次低位字节
    unsigned char Crcbyte2 = Crc & 0xFF;         // 获取低位字节

    unsigned char SingalBinSizebyte1 = (SingalBinSize >> 24) & 0xFF; // 获取高位字节
    unsigned char SingalBinSizebyte2 = (SingalBinSize >> 16) & 0xFF; // 获取次高位字节
    unsigned char SingalBinSizebyte3 = (SingalBinSize >> 8) & 0xFF;  // 获取次低位字节
    unsigned char SingalBinSizebyte4 = SingalBinSize & 0xFF;         // 获取低位字节

    unsigned char Numberbyte1 = (Number >> 8) & 0xFF;  // 获取次低位字节
    unsigned char Numberbyte2 = Number & 0xFF;         // 获取低位字节

    unsigned char headframe[] = {0x55,0xaa,AppSizebyte1,AppSizebyte2,AppSizebyte3,AppSizebyte4,Crcbyte1,Crcbyte2,
                                 SingalBinSizebyte1,SingalBinSizebyte2,SingalBinSizebyte3,SingalBinSizebyte4,Numberbyte1,
                                 Numberbyte2,BinFrameOtherLength};

    unsigned short headFrameCrc = crc16_modbus(headframe,sizeof(headframe));

    unsigned char headFrameCrcByte1 = (headFrameCrc >> 8) & 0xFF;
    unsigned char headFrameCrcByte2 = headFrameCrc & 0xFF;

    unsigned char headframewithcrc[] = {0x55,0xaa,AppSizebyte1,AppSizebyte2,AppSizebyte3,AppSizebyte4,Crcbyte1,Crcbyte2,
                                        SingalBinSizebyte1,SingalBinSizebyte2,SingalBinSizebyte3,SingalBinSizebyte4,Numberbyte1,
                                        Numberbyte2,BinFrameOtherLength,headFrameCrcByte1,headFrameCrcByte2};

    memcpy(headdata,headframewithcrc,sizeof(headframewithcrc));
}

void DataDeal::EndFrameData(unsigned char *enddata)
{
    unsigned char endframe[] = {0x77,0xcc};
    unsigned short endframeCrc = crc16_modbus(endframe,sizeof(endframe));
    unsigned char endframeCrcByte1 = (endframeCrc >> 8) & 0xFF;
    unsigned char endframeCrcByte2 = endframeCrc & 0xFF;
    unsigned char endframewithcrc[] = {0x77,0xcc,endframeCrcByte1,endframeCrcByte2};
    memcpy(enddata,endframewithcrc,sizeof(endframewithcrc));
}

void DataDeal::CommandFrameData(unsigned char *cmddata,unsigned char cmd)
{
    unsigned char cmdframe[] = {0x88,0xdd,cmd};
    unsigned short cmdframeCrc = crc16_modbus(cmdframe,sizeof(cmdframe));
    unsigned char cmdframeCrcByte1 = (cmdframeCrc >> 8) & 0xFF;
    unsigned char cmdframeCrcByte2 = cmdframeCrc & 0xFF;
    unsigned char cmdframewithcrc[] = {0x88,0xdd,cmd,cmdframeCrcByte1,cmdframeCrcByte2};
    memcpy(cmddata,cmdframewithcrc,sizeof(cmdframewithcrc));
}
