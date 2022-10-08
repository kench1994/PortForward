#include "mainwindow.h"
#include <QApplication>
#include "GuiThreadRun.hpp"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 在Gui线程创建GuiThreadRun全局对象
    GuiThreadRun::inst();
    MainWindow w;
    w.show();
    return a.exec();
}
