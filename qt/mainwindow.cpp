#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringList>
#include <QMessageBox>
#include <QToolTip>
#include <QDebug>
#include "utils/io_service_pool.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_bAddItem(false)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *pHorizontalHeader = ui->tableWidget->horizontalHeader();
    pHorizontalHeader->setSectionResizeMode(QHeaderView::Stretch);
    pHorizontalHeader->setMinimumHeight(60);
    ui->tableWidget->setHorizontalHeaderLabels({u8"监听端口", u8"下游服务器", u8"连接数", u8"实时流量", u8"状态"});
    //隐藏纵向表头
    ui->tableWidget->verticalHeader()->setHidden(true);
    //设置选择行为，以行为单位
    //ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置选择模式，选择单行
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    //设置表格字体
    ui->tableWidget->setFont(QFont("Microsoft YaHei", 16)); 

    
    ui_init();

    //启动线程池
	utils::io_service_pool::instance().run();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ui_init()
{
    //读取配置文件

    //查看当前行数
    int nRowCnt = ui->tableWidget->rowCount();
    switch(nRowCnt){
        case 0:
            //添加可用，其他按钮不可用
            setBtnState(ui->pushButtonAdd, true);
            for(auto& itBtn : {ui->pushButtonDel, ui->pushButtonOk, ui->pushButtonCancel})
                setBtnState(&*itBtn, false);
            break;
        default:
            //添加 删除可用，其他不可用
            for(auto& itBtn : {ui->pushButtonDel, ui->pushButtonAdd} )
                setBtnState(&*itBtn, true);
            for(auto& itBtn : { ui->pushButtonOk, ui->pushButtonCancel})
                setBtnState(&*itBtn, false);
            break;
    }
}

void MainWindow::setBtnState(QPushButton* pBtn, bool bEnable)
{
    if(!pBtn)
        return;
    pBtn->setEnabled(bEnable);
    if(bEnable){
        pBtn->setStyleSheet("QPushButton {"
           "	font: 700;"
           "	font-size:35px;"
           "	color:white;"
           "	border-radius:25px;"
           "    border-width:3px;"
           "	border-style:solid;"
           "	border-color:white;"
           "	background-color:transparent;"
           "}"
           "QPushButton:hover {"
           "	font: 700;"
           "	font-size:35px;"
           "	color: rgb(128, 128, 128);"
           "	border-radius:25px;"
           "    border-width:3px;"
           "	border-style:solid;"
           "	background-color:white;"
           "}"
           "QPushButton:pressed {"
           "	font: 700;"
           "	font-size:35px;"
           "	color:white;"
           "	border-color:white;"
           "    border-width:0px;"
           "	border-style:solid;"
           "	background-color:rgb(128, 128, 128);"
           "}"
        );
    }else{
        pBtn->setStyleSheet("QPushButton {"
           "	font: 700;"
           "	font-size:35px;"
           "	color:rgb(128, 128, 128);"
           "	border-radius:25px;"
           "    border-width:3px;"
           "	border-style:solid;"
           "	border-color:rgb(128, 128, 128);"
           "	background-color:transparent;"
           "}"
           "QPushButton:hover {"
           "	font: 700;"
           "	font-size:35px;"
           "	color: rgb(128, 128, 128);"
           "	border-radius:25px;"
           "    border-width:3px;"
           "	border-style:solid;"
           "	background-color:white;"
           "}"
           "QPushButton:pressed {"
           "	font: 700;"
           "	font-size:35px;"
           "	color:white;"
           "	border-color:white;"
           "    border-width:0px;"
           "	border-style:solid;"
           "	background-color:rgb(128, 128, 128);"
           "}"
        );
    }
}

void MainWindow::changeBtnState(QPushButton* pBtn)
{
        if(!pBtn)
            return;
        //禁用按钮
        if(pBtn->isEnabled()){
            setBtnState(pBtn, false);
            return;
        }
        //启用按钮
        setBtnState(pBtn, true);
    }

void MainWindow::on_pushButtonAdd_clicked()
{
    m_bAddItem = true;
    for(auto& itBtn : {ui->pushButtonAdd, ui->pushButtonDel})
        setBtnState(&*itBtn, false);
    for(auto& itBtn : {ui->pushButtonOk, ui->pushButtonCancel})
        setBtnState(&*itBtn, true);


    int nRow = ui->tableWidget->rowCount();
    //总行数增加1
    ui->tableWidget->setRowCount(nRow + 1);

    QTableWidgetItem *pItemBindPort = nullptr; 
    for(auto nColIdx = 0; nColIdx < 5; nColIdx++){

        QTableWidgetItem *pItem = new QTableWidgetItem();
        if(!pItemBindPort)
            pItemBindPort = pItem;

        ui->tableWidget->setItem(nRow, nColIdx, pItem);

        if(2 > nColIdx)
            continue;
        //不可编辑
        pItem->setFlags(pItem->flags() & (~Qt::ItemIsEditable));
    }
    //设置单元格处于编辑状态
    ui->tableWidget->setFocus();
    ui->tableWidget->editItem(pItemBindPort);
}


void MainWindow::on_pushButtonOk_clicked()
{
    unsigned int uBindPort = 0;
    const char* pszInfoRole = nullptr;
    QString qstrHost, qstrBindPort;
    int nRowIdx = ui->tableWidget->rowCount() - 1;
    for(auto nColIdx = 0; nColIdx < 2; nColIdx++){
        pszInfoRole = u8"监听端口";
        QString* pFillTarget = &qstrBindPort;
        switch (nColIdx) {
        case 1:
            pFillTarget = &qstrHost;
            pszInfoRole = u8"下游服务器";
            break;
      }

        auto *pItem = ui->tableWidget->item(nRowIdx, nColIdx);
        if(!pItem){
            pItem = new QTableWidgetItem();
            //添加到界面
            ui->tableWidget->setItem(nRowIdx, nColIdx, pItem);
        }
        if(pItem->text().isEmpty()){
            QMessageBox::information(this, "information", QString(pszInfoRole) + "为空");
            return;
        }
        *pFillTarget = pItem->text();
    }
    
    if(0 == (uBindPort = qstrBindPort.toUInt())){
        //非法端口
        QMessageBox::information(this, "information", QString::fromUtf8(pszInfoRole) + "非法输入");
        return;
    }

    //TODO:判断其他参数合法
    auto qstrListHostInfo = qstrHost.split(":");
    if(2 != qstrListHostInfo.size()) {
        QMessageBox::information(this, "information", QString::fromUtf8(pszInfoRole) + "非法输入");
        return;
    }

    //调用接口
    auto spForwarder = std::make_shared<forwarder>(uBindPort, \
        qstrListHostInfo.at(0).toStdString().data(), qstrListHostInfo.at(1).toStdString().data());

    std::unique_lock<std::mutex> lck(m_mtxForward);
    auto itF = m_mapForwards.find(uBindPort);
    if(itF != m_mapForwards.end()){
        QMessageBox::information(this, "information", "监听端口已存在");
        return;
    }

    m_mapForwards[uBindPort] = spForwarder;
    lck.unlock();

    spForwarder->setNotifyConnCnt([this](unsigned int uListenPort, unsigned int uConnCnt){
        int nRowCnt = ui->tableWidget->rowCount();
        if(0 > nRowCnt)
            return;
        //从表格中找到当前端口
        QString qstrBindPort = QString::number(uListenPort);
        for(auto i = 0; i < nRowCnt; i++) {
            auto pItem = ui->tableWidget->item(i, 0);
            if(!pItem)
                continue;
            //TODO:确认当前连接不是编辑中的连接
            if(pItem->text() != qstrBindPort)
                continue;
            pItem = ui->tableWidget->item(i, 2);
            if(!pItem)
                continue;
            pItem->setText(QString::number(uConnCnt));
            return;
        }
    });


    //结束编辑状态
    m_bAddItem = false;

    //UI flash
    ui_init();
    // for(auto& itBtn : {ui->pushButtonAdd, ui->pushButtonDel})
    //     setBtnState(&*itBtn, false);
    // for(auto& itBtn : {ui->pushButtonOk, ui->pushButtonCancel})
    //     setBtnState(&*itBtn, true);

    //处于不可编辑状态
    for(auto nColIdx = 0; nColIdx < 2; nColIdx++){
        auto *pItem = ui->tableWidget->item(nRowIdx, nColIdx);
        if(!pItem) 
            continue;
	    pItem->setFlags(pItem->flags() & (~Qt::ItemIsEditable));
    }

    for(unsigned int nColIdx = 2; nColIdx < 5; nColIdx++)
    {
        //TODO:其他几行不可被编辑,然后初始化时就assign
        QTableWidgetItem *pItem = new QTableWidgetItem();
        ui->tableWidget->setItem(nRowIdx, nColIdx, pItem); 
        switch (nColIdx)
        {
            case 2:
                pItem->setText(u8"0");
                break;
            case 3:
                pItem->setText(u8"null");
                break;
            case 4:
                pItem->setText(u8"启动中");
                break;
            default:
                break;
        }
    }
    

    //取消焦点
    ui->tableWidget->setCurrentItem(NULL);

    auto pItem = ui->tableWidget->item(nRowIdx, 4);
    if(!pItem)
        return;

    if(0 != spForwarder->start()){
        pItem->setText(u8"启动失败");
        return;
    }
    pItem->setText(u8"启动成功");
}


void MainWindow::on_pushButtonCancel_clicked()
{
    int nRowIdx = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->removeRow(nRowIdx);

    ui_init();

    m_bAddItem = false;
}


void MainWindow::on_pushButtonDel_clicked()
{
    auto nSelRow = ui->tableWidget->currentRow();
    if(-1 == nSelRow)
        return;

    auto *pItem = ui->tableWidget->item(nSelRow, 0);
    if(pItem) {
        std::shared_ptr<forwarder> spForwarder = nullptr;
        auto uBindPort = pItem->text().toUInt();
        std::unique_lock<std::mutex> lck(m_mtxForward);
        auto itF = m_mapForwards.find(uBindPort);
        if(itF != m_mapForwards.end()){
            spForwarder = std::move(itF->second);
            m_mapForwards.erase(itF);
        }
        lck.unlock();
        if(spForwarder)
            spForwarder->stop();
    }

    ui->tableWidget->removeRow(nSelRow);

    ui_init();
}

