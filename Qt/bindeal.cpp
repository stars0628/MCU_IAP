#include "bindeal.h"
#include "datadeal.h"
#include <QDebug>

#include "string.h"

static DataDeal datadeal;

BinDeal::BinDeal()
{

}

int BinDeal::Bin_Deal(QString file, unsigned int size, QString save_file, Ui::MainWindow *a)  // 对bin文件进行分割操作
{
    QByteArray input_file = file.toUtf8();
    const char *output_prefix = "bin_part";
    QByteArray output_path = save_file.toUtf8();  // 输出文件夹路径
    clearFolder(output_path);
    unsigned int chunk_size = size-6;
    BinNumber = 0;
    FILE *input_fp = fopen(input_file, "rb");
    if (input_fp == NULL) {
        printf("Could not open input file.\n");
        return 1;
    }
    // 获取原始文件大小
    fseek(input_fp, 0, SEEK_END);
    BinSize = ftell(input_fp);
    rewind(input_fp);

    a->label_5->setText("升级包大小：" + QString::number(BinSize));

    uint8_t *binbuffer = (uint8_t *)malloc(BinSize);
    fread(binbuffer, 1, BinSize, input_fp);
    AppCrc = datadeal.crc16_modbus(binbuffer,BinSize);
    free(binbuffer);

    rewind(input_fp);
    uint8_t *buffer = (uint8_t *)malloc(chunk_size);
    char output_filename[2048];
    FILE *output_fp;
    while (!feof(input_fp)) {
        size_t read_bytes = fread(buffer, 1, chunk_size, input_fp);
        if (read_bytes == 0) {
            break;
        }
        QByteArray bin_out_array = (save_file + "/%s.%d.bin").toUtf8();
        snprintf(output_filename, 2048, bin_out_array, output_prefix, BinNumber++);  // 修改生成的文件名，将路径包含在内
        output_fp = fopen(output_filename, "wb");
        if (output_fp == NULL) {
            printf("Could not create output file.\n");
            return 1;
        }

        unsigned char No_H = (BinNumber)>>8;
        unsigned char No_L = (BinNumber)&0xff;
        unsigned char addhead[] = {0x66,0xbb,No_H,No_L};

        fwrite(addhead,1,4,output_fp);
        fwrite(buffer, 1, read_bytes, output_fp);

        unsigned char *alldata = (unsigned char *)malloc(4+read_bytes);
        memcpy(alldata,addhead,4);
        memcpy(alldata+4,buffer,read_bytes);
        unsigned short crc = datadeal.crc16_modbus(alldata,4+read_bytes);
        free(alldata);
        //fwrite(&crc, 1, sizeof(crc), output_fp);

        unsigned char crch = crc>>8&0xFF;
        unsigned char crcl = crc&0xFF;
        fwrite(&crch, 1, 1, output_fp);
        fwrite(&crcl, 1, 1, output_fp);

        fclose(output_fp);
    }

    fclose(input_fp);
    free(buffer);
    return 0;
}

void BinDeal::clearFolder(const QString &folderPath)  // 清除路径下的文件和文件夹
{
    QDir folder(folderPath);
    if (folder.exists()) {
        Q_FOREACH (QFileInfo fileInfo, folder.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
            if (fileInfo.isDir()) {
                QDir(fileInfo.absoluteFilePath()).removeRecursively();
            } else {
                QFile(fileInfo.absoluteFilePath()).remove();
            }
        }
    }
}
