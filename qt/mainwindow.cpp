#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *pHorizontalHeader = ui->tableWidget->horizontalHeader();
    pHorizontalHeader->setSectionResizeMode(QHeaderView::Stretch);
    pHorizontalHeader->setMinimumHeight(60);
    pHorizontalHeader->setFont(QFont("Microsoft YaHei UI", 17));
    ui->tableWidget->setHorizontalHeaderLabels({u8"监听端口", u8"下游服务器", u8"运行状态", u8"启动/暂停"});

    //隐藏纵向表头
    ui->tableWidget->verticalHeader()->setHidden(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

