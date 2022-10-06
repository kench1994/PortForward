#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidgetItem>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <map>
#include "Forwarder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void updateBtns();

    //改变按钮状态
    void changeBtnState(QPushButton* pBtn);

    void setBtnState(QPushButton* pBtn, bool bEnable);

	void saveConfig();

	void loadConfig();

	QTableWidgetItem* addInitRow(const char* pszBindPort = nullptr, const char* pszServer = nullptr, \
	 const char* pszConnCnt = nullptr, const char* pszSpeed = nullptr, const char* pszStatus = nullptr,\
	 bool bNoneEditable = false);

	QString getItemText(const int nRow, const int nCol);

	QTableWidgetItem* getItem(const int nRow, const int nCol);

	std::shared_ptr<Forwarder> getForwarder(const unsigned int uBindPort);

	std::shared_ptr<Forwarder> addForwarder(const unsigned int uBindPort, const char* pszBindPort, const char* pszServer);

private slots:
    void on_pushButtonAdd_clicked();

    void on_pushButtonOk_clicked();

    void on_pushButtonCancel_clicked();

    void on_pushButtonDel_clicked();

    void on_pushButtonStart_clicked();

    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
    bool m_bAddItem;

    Ui::MainWindow *ui;

    std::mutex m_mtxForward;
	std::unordered_map<unsigned int, std::shared_ptr<Forwarder>> m_mapForwards;
};
#endif // MAINWINDOW_H
