//teach.h
#ifndef TEACH_H
#define TEACH_H
//
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
//
#include <QDateTime>
#include <QDebug>
#include <QDomDocument>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFile>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QPushButton>
#include <QTimer>
#include <QTextEdit>
#include <QTextStream>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>
#include <QMetaType>


//----运行按钮线程worker.h--------
#include "worker.h"
//
#include "idebug.h"
#include "motorinfo.h"

//----define--------
#define CAN_ID_SHIFT_BITS 5
#define DISABLE_MOTOR 1
#define ENABLE_MOTOR 2
//电机CMD
#define ENABLE_ALL_MOTOR 1
#define DISABLE_ALL_MOTOR 2
#define SET_TARGET 3
//比例阀CMD
#define OPEN_LOOP 1//气压开环控制

class worker;

namespace Ui {
class teach;
}

class teach : public QWidget
{
    Q_OBJECT

public:
    explicit teach(QWidget *parent = nullptr);
    ~teach();
    void appendToTextEdit(const QString &text);
    void run_SendWaypoints();
    //QVector<QVariantMap> waypointsData; // 用于运行按钮按下后，存储 waypoints 数据
    //QVector<QListWidgetItem> waypointsData;
    QVector<QStringList> waypointsData; // 用于运行按钮按下后，存储 waypoints 数据
    // Setter 方法，用于设置 IDebug 指针
    // void setIDebug(IDebug* idebug);
    
public slots:
    //查询电机状态
    void updateMotorIsOnline(bool motor1status, bool motor2status, bool motor3status, bool motor4status);

private:
    Ui::teach *ui;
    QProcess *process; // 添加 QProcess 成员变量
    int waypointCount;//worker线程中使用
    bool loop;//worker线程中使用
    // 检查CAN接口是否启动
    //bool isCanInterfaceUp();
    //运行按钮相关变量
        bool isRunning;
        int currentWaypointIndex;
        // bool isWaypointReached;         // 标志位，表示 waypoint 是否已到达
        // QTimer *waypointTimer;          // 定时器，用于超时控制
        QThread *workerThread;
        worker* worker_ptr; 
        
        //void processStopBackToZero();
        bool waitForDelayWithCondition(double delaySeconds);

private slots:
    
    // 其他槽函数
    //void on_pushButton_send_clicked();
    void on_pushButton_motor_left_1_send_clicked();
    void on_pushButton_motor_left_2_send_clicked();
    void on_pushButton_motor_right_1_send_clicked();
    void on_pushButton_motor_right_2_send_clicked();
    void on_pushButton_arm_right_send_clicked();
    void on_pushButton_arm_left_send_clicked();



    void on_pushButton_motor_left_1_toZero_clicked();
    void on_pushButton_motor_left_2_toZero_clicked();
    void on_pushButton_motor_right_1_toZero_clicked();
    void on_pushButton_motor_right_2_toZero_clicked();  
    void on_pushButton_arm_right_toZero_clicked();
    void on_pushButton_arm_left_toZero_clicked();

    //void on_pushButton_SetCan0Up_clicked();
    //void on_pushButton_SetCan0Down_clicked();

    //void run_SendWaypoints();
    void sendWaypointData(const QStringList &values);
    void processStopBackToZero();
    //电机CAN发送按钮辅助函数
    void sendCanMessage(int      canID,
                        uint32_t targetAngle,
                        uint16_t targetVelocity);
    void processInput(QLineEdit *idLineEdit,
                      QLineEdit *positionLineEdit,
                      QLineEdit *velocityLineEdit);

    void processInput_valve(bool isLeftArm,
                            QLineEdit *LineEdit_arm_left_j1_m1,
                            QLineEdit *LineEdit_arm_left_j1_m2,
                            QLineEdit *LineEdit_arm_left_j1_m3,
                            QLineEdit *LineEdit_arm_left_j1_m4,
                            QLineEdit *LineEdit_arm_left_j2_m1,
                            QLineEdit *LineEdit_arm_left_j2_m2,
                            QLineEdit *LineEdit_arm_left_j2_m3,
                            QLineEdit *LineEdit_arm_left_j2_m4,
                            QLineEdit *LineEdit_arm_left_j3_m1,
                            QLineEdit *LineEdit_arm_left_j3_m2,
                            QLineEdit *LineEdit_arm_left_j3_m3,
                            QLineEdit *LineEdit_arm_left_j3_m4,
                            QLineEdit *LineEdit_arm_left_j4_m1,
                            QLineEdit *LineEdit_arm_left_j4_m2,
                            QLineEdit *LineEdit_arm_left_j4_m3,
                            QLineEdit *LineEdit_arm_left_j4_m4,
                            QLineEdit *LineEdit_pressure_limit);

    void sendCanMessage_valve(bool isLeftArm, int jointid, uint16_t pressure[4]);
    void logCanMessage_motor(int canID, uint32_t targetAngle, uint16_t targetVelocity);
    void logCanMessage_valve(bool isLeftArm, uint16_t pressures[4][4]);
    void logMessage(const QString log);
    //运行按钮线程函数
    void handleWorkerFinished();
    void handleLogMessage(const QString &message);
    
};
//
#endif // TEACH_H
