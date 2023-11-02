#ifndef BINDEAL_H
#define BINDEAL_H

#include <QFileDialog>
#include "ui_mainwindow.h"

class BinDeal
{
public:
    BinDeal();
    int Bin_Deal(QString file, unsigned int size, QString save_file, Ui::MainWindow *a);
    void clearFolder(const QString &folderPath);



    QString BinFilePath;
    QString OutBinFilePath;
    unsigned short BinSize;
    unsigned short AppCrc;
    unsigned int SignalBinSize;
    unsigned short BinNumber;
};

#endif // BINDEAL_H
