#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void HeadDataSend();
    void BinDataSendReady(unsigned short No);
    void EndDataSend();
    void CommandSend(unsigned char cmd);
    QByteArray readFile(const QString &filePath);
    void UpdateOverTime();
    void DataSendSuccess(unsigned char type);
    void SerialDataDeal(unsigned short head,unsigned char type);
    void SerialSend(QByteArray array);
    void EndAction();

public slots:
    void SerialPortReadyRead_slot();

private slots:
    void on_connectBtn_clicked();
    bool eventFilter(QObject *watched, QEvent *event);
    void on_FileSelect_clicked();
    void on_UpdataStart_clicked();
    void on_Stop_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
