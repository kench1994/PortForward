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
    //pHorizontalHeader->setFont(QFont("Microsoft YaHei UI", 14));
    ui->tableWidget->setHorizontalHeaderLabels({u8"监听端口", u8"下游服务器", u8"连接数", u8"实时流量", u8"启动/暂停"});
    ui->tableWidget->setStyleSheet(
            "QTableWidget{ color:black; \
                             background-color:rgb(128,128,128);\
                             selection-color:rgb(0, 0, 0); \
                             selection-background-color:rgb(	100,149,237);\
                             border:0px;\
                             font: 8pt 'Microsoft YaHei' ;}"
            "QHeaderView::section:horizontal{ 	/*设置标题(水平的)*/\
                            border: 0px solid rgb(255, 255, 255); 	/*白色间隔*/\
                            border - bottom: 0px;/*下边框不需要颜色*/\
                            color: rgb(255, 255, 255);\
                            background: rgb(128, 0, 128);\
                            padding - left: 2px;\
                            font: 700 14pt 'Microsoft YaHei UI';\
                            min - width:60px;}"
    );
    //隐藏纵向表头
    ui->tableWidget->verticalHeader()->setHidden(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

