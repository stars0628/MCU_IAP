#ifndef DATADEAL_H
#define DATADEAL_H

class DataDeal
{
public:
    DataDeal();
    unsigned short crc16_modbus(unsigned char *data, unsigned int len);
    void HeadFrameData(unsigned int AppSize,unsigned short Crc,unsigned int SingalBinSize,unsigned short Number,unsigned char BinFrameOtherLength,unsigned char *headdata);
    void EndFrameData(unsigned char *enddata);
    void CommandFrameData(unsigned char *cmddata,unsigned char cmd);
};

#endif // DATADEAL_H
