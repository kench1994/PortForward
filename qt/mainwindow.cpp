#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringList>
#include <QMessageBox>
#include <QToolTip>
#include <QDebug>
#include <fstream>
#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>
#include "utils/io_service_pool.hpp"
#include "GuiThreadRun.hpp"
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
    //选择行为单位
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //选择单行
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        
	loadConfig();

	updateBtns();

    //启动线程池
	utils::io_service_pool::instance().run();
}

MainWindow::~MainWindow()
{
	//TODO:
    delete ui;
}

void MainWindow::updateBtns()
{
    //读取配置文件

    //查看当前行数
    int nRowCnt = ui->tableWidget->rowCount();
    switch(nRowCnt){
        case 0:
            //添加可用，其他按钮不可用
            setBtnState(ui->pushButtonAdd, true);
            for(auto& itBtn : {ui->pushButtonDel, ui->pushButtonOk, ui->pushButtonCancel, ui->pushButtonStart})
                setBtnState(&*itBtn, false);
            break;
        default:
            //添加 删除可用，其他不可用
            for(auto& itBtn : {ui->pushButtonDel, ui->pushButtonAdd, ui->pushButtonStart} )
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
		if (pBtn == ui->pushButtonStart) {
			return;
		}
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
		if (pBtn == ui->pushButtonStart) {
			return;
		}
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

void MainWindow::saveConfig()
{
	//读取当前表格中的状态
	if (m_bAddItem)
		return;

	nlohmann::json jsonArray;
	int nRowCnt = ui->tableWidget->rowCount();
	for (int nRowIdx = 0; nRowIdx < nRowCnt; nRowIdx++) {
		nlohmann::json singleJsonObj;
		//读取监听端口,下游服务器,状态
		{
			auto qstrPort = getItemText(nRowIdx, 0);
			if (qstrPort.isEmpty() || 0 == qstrPort.toUInt())
				continue;
			singleJsonObj["port"] = qstrPort.toUtf8();
		}
		{
			auto qstrServ = getItemText(nRowIdx, 1);
			if (qstrServ.isEmpty())
				continue;
			singleJsonObj["server"] = qstrServ.toUtf8();
		}
		{
			//fnGetItemText(nRowIdx, 2);
		}
		jsonArray.emplace_back(std::move(singleJsonObj));
	}

	std::string strCtx = jsonArray.dump(4);

	//写入到配置文件中
	std::ofstream ofs("forward.json", std::ios::ate);
	ofs.write(strCtx.data(), strCtx.length());
	ofs.close();
}

void MainWindow::loadConfig()
{
	std::ifstream ifs("forward.json", std::ios::binary);
	std::string strCtx((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();

	try {
		auto jsonArray = nlohmann::json::parse(strCtx);
		for (const auto& itJsonObj : jsonArray) {
			try {
				auto strPort = itJsonObj["port"].get<std::string>();
				auto strServer = itJsonObj["server"].get<std::string>();
				auto uPort = static_cast<unsigned int>(atoi(strPort.data()));
				if (0 == uPort)
					continue;

				std::vector<std::string> vHostInfo;
				boost::split(vHostInfo, strServer, boost::is_any_of(":"), boost::token_compress_on);
				if (2 != vHostInfo.size())
					continue;

				addInitRow(strPort.data(), strServer.data(), "0", "未实现", "未启动", true);
				addForwarder(uPort, vHostInfo[0].data(), vHostInfo[1].data());
			} catch (...) {

			}
		}
	} catch (...) {

	}
}

QTableWidgetItem* MainWindow::addInitRow(const char* pszBindPort, const char* pszServer, \
	const char* pszConnCnt, const char* pszSpeed, const char* pszStatus, bool bNoneEditable)
{
	int nRow = ui->tableWidget->rowCount();
	//总行数增加1
	ui->tableWidget->setRowCount(nRow + 1);
	static QFont font("Microsoft YaHei UI", 10.5);

	QTableWidgetItem *pFirstItem = nullptr;
	for (auto nColIdx = 0; nColIdx < 5; nColIdx++) {

		QTableWidgetItem* pItem = new QTableWidgetItem();
		pItem->setFont(font);
		pItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		bool bEditable = false;
		switch (nColIdx)
		{
		case 0:
			bEditable = true;
			if (pszBindPort)
				pItem->setText(QString::fromUtf8(pszBindPort));
			pFirstItem = pItem;
			break;
		case 1:
			if (pszServer)
				pItem->setText(QString::fromUtf8(pszServer));
			bEditable = true;
			break;
		case 2:
			if (pszConnCnt)
				pItem->setText(QString::fromUtf8(pszConnCnt));
			break;
		case 3:
			if (pszSpeed)
				pItem->setText(QString::fromUtf8(pszSpeed));
			break;
		case 4:
			if (pszStatus)
				pItem->setText(QString::fromUtf8(pszStatus));
			break;
		}

		ui->tableWidget->setItem(nRow, nColIdx, pItem);

		//不可编辑
		if (!bEditable || bNoneEditable) {
			//| Qt::ItemIsEnabled | Qt::ItemIsSelectable
			pItem->setFlags(pItem->flags() & (~Qt::ItemIsEditable));
			continue;
		}
	}
	//设置行高
	ui->tableWidget->setRowHeight(nRow, 40);
	ui->tableWidget->setColumnWidth(0, ui->tableWidget->columnWidth(0));
	return pFirstItem;
}

std::shared_ptr<Forwarder> MainWindow::addForwarder(const unsigned int uBindPort, const char* pszServerIp, const char* pszServerPort)
{
	auto spForwarder = std::make_shared<Forwarder>(uBindPort, \
		pszServerIp, pszServerPort);

	std::unique_lock<std::mutex> lck(m_mtxForward);
	auto itF = m_mapForwards.find(uBindPort);
	if (itF != m_mapForwards.end()) {
		QMessageBox::information(this, "information", "监听端口已存在");
		return nullptr;
	}

	m_mapForwards[uBindPort] = spForwarder;
	lck.unlock();

	spForwarder->setNotifyConnCnt([this](unsigned int uListenPort, unsigned int uConnCnt) {
		qDebug() << "listenPort:" << uListenPort << ",curr conn cnt:" << uConnCnt;
		int nRowCnt = ui->tableWidget->rowCount();
		if (0 > nRowCnt)
			return;
		//从表格中找到当前端口
		QString qstrBindPort = QString::number(uListenPort);
		for (auto i = 0; i < nRowCnt; i++) {
			auto pItem = ui->tableWidget->item(i, 0);
			if (!pItem)
				continue;
			//TODO:确认当前连接不是编辑中的连接
			if (pItem->text() != qstrBindPort)
				continue;
			pItem = ui->tableWidget->item(i, 2);
			if (!pItem)
				continue;
			//TODO:在主线程中更新UI
			fprintf(stdout, "conn cnt:%d\r\n", uConnCnt);
			QString qstrConnNum = QString::number(uConnCnt);

			auto updateConnCntShow = [pItem, qstrConnNum]() {
				pItem->setText(qstrConnNum);
			};
			GuiThreadRun::inst().excute(updateConnCntShow);
			return;
		}
	});

	return spForwarder;
}

QString MainWindow::getItemText(const int nRow, const int nCol)
{
	QString qstrResult;
	auto pItem = ui->tableWidget->item(nRow, nCol);
	if (!pItem)
		return qstrResult;
	return pItem->text();
}

QTableWidgetItem* MainWindow::getItem(const int nRow, const int nCol)
{
	auto pItem = ui->tableWidget->item(nRow, nCol);
	return pItem;
}

std::shared_ptr<Forwarder> MainWindow::getForwarder(const unsigned int uBindPort)
{
	std::lock_guard<std::mutex> lck(m_mtxForward);
	auto itF = m_mapForwards.find(uBindPort);
	if (itF == m_mapForwards.end())
		return nullptr;
	auto spForwarder = itF->second;
	return spForwarder;
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

	auto *pItem = addInitRow(nullptr, nullptr, "0", "未实现", "未启动");
	//设置焦点
	ui->tableWidget->setFocus();
	//设置编辑中
	ui->tableWidget->editItem(pItem);

}


void MainWindow::on_pushButtonOk_clicked()
{
    unsigned int uBindPort = 0;
    QString qstrHost, qstrBindPort;
    int nRowIdx = ui->tableWidget->rowCount() - 1;

    for(auto nColIdx = 0; nColIdx < 2; nColIdx++){
        const char* pszInfoRole = u8"监听端口";
        QString* pFillTarget = &qstrBindPort;
        switch (nColIdx) {
            case 1:
                pFillTarget = &qstrHost;
                pszInfoRole = u8"下游服务器";
                break;
        }

        auto *pItem = ui->tableWidget->item(nRowIdx, nColIdx);
        if(pItem->text().isEmpty()){
            QMessageBox::information(this, "information", QString(pszInfoRole) + "为空");
            return;
        }
        *pFillTarget = pItem->text();
    }
    
    if(0 == (uBindPort = qstrBindPort.toUInt())){
        //非法端口
        QMessageBox::information(this, "information", "监听端口非法输入");
        return;
    }

    //TODO:判断其他参数合法
    auto qstrListHostInfo = qstrHost.split(":");
    if(2 != qstrListHostInfo.size()) {
        QMessageBox::information(this, "information", "下游服务器非法输入");
        return;
    }

	auto spForwarder = addForwarder(uBindPort, qstrListHostInfo[0].toUtf8().data(), qstrListHostInfo[1].toUtf8().data());


    //结束编辑状态
    m_bAddItem = false;

    //UI flash
    updateBtns();
  
	//不可编辑
	for (auto i = 0; i < 2; i++) {
		auto pItem = ui->tableWidget->item(nRowIdx, i);
		if(pItem)
			pItem->setFlags(pItem->flags() & (~Qt::ItemIsEditable));
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

	saveConfig();
}


void MainWindow::on_pushButtonCancel_clicked()
{
    int nRowIdx = ui->tableWidget->rowCount() - 1;

    ui->tableWidget->removeRow(nRowIdx);

    updateBtns();

    m_bAddItem = false;
}


void MainWindow::on_pushButtonDel_clicked()
{
    auto nSelRow = ui->tableWidget->currentRow();
    if(-1 == nSelRow)
        return;

    auto *pItem = ui->tableWidget->item(nSelRow, 0);
    if(pItem) {
        std::shared_ptr<Forwarder> spForwarder = nullptr;
        auto uBindPort = pItem->text().toUInt();
        std::unique_lock<std::mutex> lck(m_mtxForward);
        auto itF = m_mapForwards.find(uBindPort);
        if(itF != m_mapForwards.end()) {
            spForwarder = std::move(itF->second);
            m_mapForwards.erase(itF);
        }
        lck.unlock();
        if(spForwarder)
            spForwarder->stop();
    }

    ui->tableWidget->removeRow(nSelRow);

	saveConfig();

	updateBtns();
}

void MainWindow::on_pushButtonStart_clicked()
{
	//根据当前的状态更改图标
	int nCurrRowIdx = ui->tableWidget->currentRow();

	//获取监听端口
	auto qstrBindPort = getItemText(nCurrRowIdx, 0);
	auto uBindPort = atoi(qstrBindPort.toUtf8());
	if (0 == uBindPort)
		return;
	//获取forwarder
	auto spForwarder = getForwarder(uBindPort);
	if (nullptr == spForwarder)
		return;

	auto pItemStatus = getItem(nCurrRowIdx, 4);
	if (0 != spForwarder->start()) {
		pItemStatus->setText(u8"启动失败");
		return;
	}
	pItemStatus->setText(u8"启动成功");
	ui->pushButtonStart->setText("||");
}


void MainWindow::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
	if (currentRow == previousRow)
		return;
	auto pItemStatus = getItem(currentRow, 4);
	if (!pItemStatus)
		return;
	if (!pItemStatus->text().compare("未启动")) {
		ui->pushButtonStart->setStyleSheet("QPushButton {"
			"	font: 700;"
			"	font-size:40px;"
			"	color:white;"
			"	border-radius:25px;"
			"   border-width:3px;"
			"	border-style:solid;"
			"	border-color: white;"
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
			"   border-width:0px;"
			"	border-style:solid;"
			"	background-color:rgb(128, 128, 128);"
		);
		ui->pushButtonStart->setText("▶");
		return;
	}
	ui->pushButtonStart->setStyleSheet("QPushButton {"
		"	font: 700;"
		"	font-size:25px;"
		"	color:white;"
		"	border-radius:25px;"
		"   border-width:3px;"
		"	border-style:solid;"
		"	border-color: white;"
		"	background-color:transparent;"
		"}"
		"QPushButton:hover {"
		"	font: 700;"
		"	font-size:25px;"
		"	color: rgb(128, 128, 128);"
		"	border-radius:25px;"
		"    border-width:3px;"
		"	border-style:solid;"
		"	background-color:white;"
		"}"
		"QPushButton:pressed {"
		"	font: 700;"
		"	font-size:25px;"
		"	color:white;"
		"	border-color:white;"
		"   border-width:0px;"
		"	border-style:solid;"
		"	background-color:rgb(128, 128, 128);"
	);
	ui->pushButtonStart->setText("||");
}

