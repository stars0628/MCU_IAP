#include <QDebug>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QStandardPaths>

#include "bindeal.h"
#include "datadeal.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <string.h>

typedef struct {
    unsigned char lastDataType;//上一个数据类型：0x00起始帧 0x01bin数据帧 0x02结束帧 0x03命令帧
    bool updating;//正在升级标志
    unsigned short nextBinNumber;
} UpdateProcess;

QSerialPort serialPort;            // 配置串口参数
QTimer *Time_wait = new QTimer();  // 用于发送升级数据后的等待时间
static DataDeal datadeal;
static BinDeal bindeal;
UpdateProcess updateprocess;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->SerialComboBox->installEventFilter(this);

    /* 接收数据信号槽 */
    connect(&serialPort, &QSerialPort::readyRead, this, &MainWindow::SerialPortReadyRead_slot);

    // 超时无应答视为发送失败
    connect(Time_wait, &QTimer::timeout, this, [=]() {
        UpdateOverTime();
    });

    ui->UpdataStart->setEnabled(false);

    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    bindeal.OutBinFilePath = documentsPath + QDir::separator() + "binTemp";
    QDir().mkpath(bindeal.OutBinFilePath);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_connectBtn_clicked()
{
    if (ui->connectBtn->text() == "打开串口") {
        QString SerialCom = ui->SerialComboBox->currentText();
        QStringList SerialComList = SerialCom.split("  ");
        serialPort.setPortName(SerialComList[0]);
        QString BaudRateS = ui->lineEdit->text();
        qint32 BaudRateInt = BaudRateS.toInt();
        serialPort.setBaudRate(BaudRateInt);
        serialPort.setDataBits(QSerialPort::Data8);
        serialPort.setParity(QSerialPort::NoParity);
        serialPort.setStopBits(QSerialPort::OneStop);
        if (!serialPort.open(QIODevice::ReadWrite)) {
            QMessageBox::warning(this, "警告", "打开串口失败");
            return;
        }
        ui->connectBtn->setText("关闭串口");
        ui->UpdataStart->setEnabled(true);
        ui->SerialComboBox->setEnabled(false);
    } else {
        serialPort.close();
        ui->connectBtn->setText("打开串口");
        ui->UpdataStart->setEnabled(false);
        ui->SerialComboBox->setEnabled(true);
    }
}

void MainWindow::on_FileSelect_clicked()
{
    // 创建文件选择对话框，设置对话框标题和默认打开的目录
    QFileDialog dialog(nullptr, "Select a file", QDir::currentPath(), "Bin (*.bin)");
    // 设置对话框打开模式为选择单个文件
    dialog.setFileMode(QFileDialog::ExistingFile);
    // 显示文件选择对话框，并等待用户的选择

    if (dialog.exec() == QDialog::Accepted) {
        // 获取用户所选文件的路径
        bindeal.BinFilePath = dialog.selectedFiles()[0];
        ui->label_3->setText("Bin文件：" + bindeal.BinFilePath);
    }
}

void MainWindow::on_UpdataStart_clicked()
{
    if(!bindeal.BinFilePath.isEmpty()){
        HeadDataSend();
        ui->UpdataStart->setEnabled(false);
    } else {
        QMessageBox::warning(this, tr("警告"), tr("请选择升级文件"), QMessageBox::Close);
        EndAction();
    }
}

void MainWindow::on_Stop_clicked()
{
    EndAction();
    CommandSend(0x00);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == ui->SerialComboBox) {
                if(ui->SerialComboBox->isEnabled()){
                QComboBox *comboBox = qobject_cast<QComboBox *>(watched);
                comboBox->clear();
                // 获取所有可用的串口信息
                QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
                // 遍历所有串口信息并添加到下拉列表中
                foreach (const QSerialPortInfo &serialPortInfo, serialPorts) {
                    ui->SerialComboBox->addItem(serialPortInfo.portName() + "  " + serialPortInfo.description());
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::SerialPortReadyRead_slot()
{
    QByteArray receivedData = serialPort.readAll();
    unsigned short datasize = receivedData.size();
    unsigned char *receivedDataU8 = (unsigned char *)malloc(datasize);
    receivedDataU8 = reinterpret_cast<unsigned char *>(receivedData.data());
    if ((datasize>2)&&((receivedDataU8[datasize - 2] << 8 | receivedDataU8[datasize - 1]) == datadeal.crc16_modbus(receivedDataU8, datasize - 2))) {
        unsigned short head = receivedDataU8[0] << 8 | receivedDataU8[1];
        SerialDataDeal(head, receivedDataU8[2]);
    } else {
        //
    }
    free(receivedDataU8);//在64位程序中这个free会导致此程序崩溃，才疏学浅，不懂
}

void MainWindow::HeadDataSend()
{
    bindeal.SignalBinSize = ui->lineEdit_2->text().toUInt();
    bindeal.Bin_Deal(bindeal.BinFilePath, bindeal.SignalBinSize, bindeal.OutBinFilePath, ui);
    unsigned char data[17];
    datadeal.HeadFrameData(bindeal.BinSize, bindeal.AppCrc, bindeal.SignalBinSize, bindeal.BinNumber,6 , data);
    QByteArray dataArray(reinterpret_cast<char *>(data), sizeof(data));
    SerialSend(dataArray);
    updateprocess.lastDataType = 0x00;
}

void MainWindow::BinDataSendReady(unsigned short No)
{
    QString BinNo;  // 发送的bin编号
    BinNo = BinNo.number(No);
    readFile(bindeal.OutBinFilePath + "/bin_part." + BinNo + ".bin");
}

void MainWindow::EndDataSend()
{
    unsigned char data[4];
    datadeal.EndFrameData(data);
    QByteArray dataArray(reinterpret_cast<char *>(data), sizeof(data));
    SerialSend(dataArray);
    updateprocess.lastDataType = 0x02;
}

void MainWindow::CommandSend(unsigned char cmd)
{
    unsigned char data[5];
    datadeal.CommandFrameData(data, cmd);
    QByteArray dataArray(reinterpret_cast<char *>(data), sizeof(data));
    SerialSend(dataArray);
    updateprocess.lastDataType = 0x03;
}

// 从指定路径读入文件
QByteArray MainWindow::readFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("警告"), tr("文件打开失败"), QMessageBox::Close);
        EndAction();
        return QByteArray();
    }
    QByteArray fileData = file.readAll();
    SerialSend(fileData);  // 发送bin文件
    updateprocess.lastDataType = 0x01;
    file.close();
    return fileData;
}

void MainWindow::UpdateOverTime()
{
    switch (updateprocess.lastDataType) {
    case 0x00:
        HeadDataSend();
        break;
    case 0x01:
        BinDataSendReady(updateprocess.nextBinNumber);
        break;
    case 0x02:
        EndDataSend();
        break;
    case 0x03:
        CommandSend(0x00);
        break;
    default:
        //
        break;
    }
}

void MainWindow::DataSendSuccess(unsigned char type)
{
    switch (type){
    case 0x00:
        Time_wait->stop();
        updateprocess.updating = true;
        BinDataSendReady(updateprocess.nextBinNumber);
        break;
    case 0x01:
        if(updateprocess.updating == true)
        {
            Time_wait->stop();
            updateprocess.nextBinNumber++;
            if(updateprocess.nextBinNumber < bindeal.BinNumber){
                BinDataSendReady(updateprocess.nextBinNumber);
            } else {
                EndDataSend();
            }

            ui->updataProgressBar->setValue(100*updateprocess.nextBinNumber/bindeal.BinNumber);
        }
        break;
    case 0x02:
        EndAction();
        QMessageBox::information(this,"信息","升级成功");
        break;
    case 0x03:
        Time_wait->stop();
        break;
    default:
        //
        break;
    }
}

void MainWindow::SerialDataDeal(unsigned short head, unsigned char type)
{
    switch (head) {
    case 0x11ee:
        DataSendSuccess(type);
        break;
    case 0x22ff:
        break;
    case 0x3344:
        break;
    }
}

void MainWindow::SerialSend(QByteArray array)
{
    serialPort.write(array);
    Time_wait->start(1000);
}

void MainWindow::EndAction()
{
    memset(&updateprocess,0,sizeof(UpdateProcess));
    Time_wait->stop();
    ui->UpdataStart->setEnabled(true);
}

/*
上位机发送下位机接收
1.传输起始帧：包含AppMessage的所有内容。
格式："55aa" + 4字节AppSize + 2字节AppCrc + 4字节SignalPackSize + 2字节TotalNumber + 1字节每个数据帧包含的额外信息长度(除了bin数据外的所有数据) + crc16-modbus
2.Bin数据帧：发送Bin的编号和Bin数据
格式："66bb" + 2字节编号+Bin段数据 + crc16-modbus
3.传输结束帧：所有bin文件传输完成发送
格式："77cc" + crc16-modbus
4.其他命令帧：00重置升级
格式："88dd" + 1个字节命令 + crc16-modbus

上位机接收下位机发送
类型：0x00起始帧 0x01bin数据帧 0x02结束帧 0x03命令帧
1.接收成功
格式："11ee" + 1字节类型 + crc16-modbus
2.校验错误
格式；"22ff" 0xff + crc16-modbus
3.数据异常
格式："3344" + 1字节类型 + crc16-modbus
*/
