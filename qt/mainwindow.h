#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTableWidgetItem>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <map>
#include "forwarder.h"

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
    void ui_init();


    //改变按钮状态
    void changeBtnState(QPushButton* pBtn);


    void setBtnState(QPushButton* pBtn, bool bEnable);

private slots:
    void on_pushButtonAdd_clicked();

    void on_pushButtonOk_clicked();

    void on_pushButtonCancel_clicked();

    void on_pushButtonDel_clicked();

private:
    bool m_bAddItem;

    Ui::MainWindow *ui;

    std::mutex m_mtxForward;
	std::unordered_map<unsigned int, std::shared_ptr<forwarder>> m_mapForwards;
};
#endif // MAINWINDOW_H
