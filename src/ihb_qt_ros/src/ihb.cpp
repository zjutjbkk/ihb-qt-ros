
#include "ihb.h"
#include "ui_ihb.h"
#include <QFile>
#include <QKeyEvent>
#include <QTabBar>
#include <QDebug>

IHB::IHB(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IHB)
{
    ui->setupUi(this);

    ui->MainTab->tabBar()->setObjectName("MainTabBar");

    //设置qss
    QFile qss("://files/ihb.qss");
    // 检查文件是否存在
    if (!qss.exists())
    {
        qDebug() << "qss file not found";
        return;
    }
    qss.open(QFile::ReadOnly);
    this->setStyleSheet(qss.readAll());

    //qrc:/files/ico/x.ico
    setWindowIcon(QIcon(":/files/ico/x.ico"));

    ui->MainTab->setCurrentIndex(3);
    

}

IHB::~IHB()
{
    delete ui;
}


void IHB::keyPressEvent(QKeyEvent *event){
    switch (event->key())
    {
    case Qt::Key_A: // 弹出关于界面
        qDebug("a");
//        if (!ui->labelAbout->isVisible())
//            ui->labelAbout->show();
//        else
//            ui->labelAbout->hide();

        break;
    case Qt::Key_Escape:
//        this->hide();
        this->close();
    default:
        break;
    }
}
