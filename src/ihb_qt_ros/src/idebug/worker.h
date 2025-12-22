// worker.h
#ifndef WORKER_H
#define WORKER_H
//------------------------------
#include <QObject>
#include <QMutex>
#include <QWaitCondition>
//------------------------------
#include "teach.h"
//------------------------------
class teach;

class worker : public QObject
{
    Q_OBJECT
public:
    explicit worker(teach *teachInstance, QObject *parent = nullptr);
    //explicit worker(QObject *parent = nullptr);
    worker();
    ~worker();

private:
    teach *m_teach;
    bool m_stop;
    QMutex m_mutex;
    QWaitCondition m_waitCondition;

public slots:
    void process(); // 执行 run_SendWaypoints
    void stop();    // 停止运行

signals:
    void finished();
    void logMessage(const QString &message);
};

#endif // WORKER_H
