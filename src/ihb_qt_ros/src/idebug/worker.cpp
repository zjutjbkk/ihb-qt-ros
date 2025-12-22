// worker.cpp
#include "worker.h"
#include "teach.h" // 确保可以访问 teach 类的成员函数
#include <QThread>

worker::worker(teach *teachInstance, QObject *parent) 
    : QObject(parent), m_teach(teachInstance), m_stop(false)
{
}

worker::~worker()
{
}

void worker::process()
{
    //emit logMessage("RUN线程开始执行");
    
    m_teach->run_SendWaypoints();
    QString strText2 = "running end";
    qDebug()<< strText2;
    // 发送完成信号
    emit finished();
}

void worker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    m_waitCondition.wakeAll(); // 唤醒等待的线程
}
