#ifndef STATICSCONTROL_H
#define STATICSCONTROL_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>
#include <QListWidget>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ihb_msgs/msg/servo_motor_joint_target.hpp"
#include "ihb_msgs/msg/muscle_joint_target.hpp"
#include "ihb_msgs/msg/stepper_motor_joint_target.hpp"  // 步进电机消息
#include "ihb_msgs/msg/arm_target.hpp"  // 机械臂目标消息 (新架构)
#include "src/connect/signaltransmitter.h"

// 宏定义
#define CLOSE_PID 0  // 关闭PID控制
#define OPEN_LOOP_with_statics 1  // 静力学模型开环控制
#define PID_CONTROL 2  // PID控制

// 路径点数据结构
struct Waypoint {
    QString name;                    // 路径点名称
    double stepperHeight;            // 步进电机高度 (新增)
    double leftServoAngles[2];       // 左臂伺服电机角度
    double leftMuscleAngles[8];      // 左臂肌肉关节角度
    double rightServoAngles[2];      // 右臂伺服电机角度
    double rightMuscleAngles[8];     // 右臂肌肉关节角度

    // 构造函数
    Waypoint() {
        name = "";
        stepperHeight = 0.0;
        for (int i = 0; i < 2; i++) {
            leftServoAngles[i] = 0.0;
            rightServoAngles[i] = 0.0;
        }
        for (int i = 0; i < 8; i++) {
            leftMuscleAngles[i] = 0.0;
            rightMuscleAngles[i] = 0.0;
        }
    }
};

// 延时指令数据结构
struct DelayCommand {
    QString name;        // 延时指令名称
    double delaySeconds; // 延时秒数

    DelayCommand() : name(""), delaySeconds(0.0) {}
    DelayCommand(double seconds) : delaySeconds(seconds) {
        name = QString("等待 %1 秒").arg(seconds);
    }
};

// 轨迹步骤类型枚举
enum StepType {
    STEP_WAYPOINT,  // 路径点
    STEP_DELAY      // 延时
};

// 统一的轨迹步骤结构
struct TrajectoryStep {
    StepType type;           // 步骤类型
    Waypoint waypoint;       // 路径点数据（当type为STEP_WAYPOINT时有效）
    double delaySeconds;     // 延时秒数（当type为STEP_DELAY时有效）

    TrajectoryStep() : type(STEP_WAYPOINT), delaySeconds(0.0) {}
};

namespace Ui {
class StaticsControl;
}

class StaticsControl : public QWidget
{
    Q_OBJECT

public:
    explicit StaticsControl(QWidget *parent = nullptr);
    ~StaticsControl();

private slots:
    // 发送按钮槽函数
    void on_pushButton_send_clicked();
    // 停止按钮槽函数 (发送停止消息 + 界面归零)
    void on_pushButton_stop_clicked();
    // 左臂发送按钮
    void on_pushButton_send_left_clicked();
    // 右臂发送按钮
    void on_pushButton_send_right_clicked();
    // 归零按钮槽函数 (仅界面归零,不发送消息)
    void on_pushButton_reset_clicked();

    // 示教功能槽函数
    void on_pushButton_record_waypoint_clicked();      // 记录路径点
    void on_pushButton_delete_waypoint_clicked();      // 删除路径点
    void on_pushButton_insert_delay_clicked();         // 插入延时
    void on_pushButton_save_trajectory_clicked();      // 保存轨迹
    void on_pushButton_load_trajectory_clicked();      // 加载轨迹
    void on_pushButton_run_trajectory_clicked();       // 运行/停止轨迹
    void on_pushButton_clear_trajectory_clicked();     // 清空轨迹
    void on_listWidget_waypoints_itemDoubleClicked(QListWidgetItem *item);  // 双击路径点加载到界面
    void on_listWidget_waypoints_itemClicked(QListWidgetItem *item);        // 单击路径点加载到界面

    // 轨迹运行相关槽函数
    void executeNextTrajectoryStep();                  // 执行下一个轨迹步骤

    // 步进电机槽函数
    void on_horizontalSlider_stepper_height_valueChanged(int value);  // 步进电机滑块值改变
    void on_doubleSpinBox_stepper_height_valueChanged(double value);  // 步进电机输入框值改变

private:
    Ui::StaticsControl *ui;

    // 左臂角度参数 (2个伺服电机 + 4个肌肉关节*2个角度 = 10个参数)
    QDoubleSpinBox *leftServoSpinBoxes[2];      // 左臂伺服电机角度
    QDoubleSpinBox *leftMuscleSpinBoxes[8];     // 左臂肌肉关节角度 (4个关节*2个角度)
    QSlider *leftServoSliders[2];               // 左臂伺服电机滑块
    QSlider *leftMuscleSliders[8];              // 左臂肌肉关节滑块

    // 右臂角度参数 (2个伺服电机 + 4个肌肉关节*2个角度 = 10个参数)
    QDoubleSpinBox *rightServoSpinBoxes[2];     // 右臂伺服电机角度
    QDoubleSpinBox *rightMuscleSpinBoxes[8];    // 右臂肌肉关节角度 (4个关节*2个角度)
    QSlider *rightServoSliders[2];              // 右臂伺服电机滑块
    QSlider *rightMuscleSliders[8];             // 右臂肌肉关节滑块

    // 示教功能相关成员变量
    QList<Waypoint> waypointList;               // 路径点列表（保留用于兼容）
    QList<DelayCommand> delayList;              // 延时指令列表（保留用于兼容）
    QList<TrajectoryStep> trajectorySteps;      // 统一的轨迹步骤列表
    int waypointCounter;                        // 路径点计数器

    // 轨迹运行相关变量
    bool isTrajectoryRunning;                   // 轨迹是否正在运行
    int currentTrajectoryIndex;                 // 当前执行的轨迹索引
    QTimer *trajectoryTimer;                    // 轨迹执行定时器

    // 辅助函数
    void setupUI();
    void createArmControlGroup(QGroupBox *groupBox,
                               QDoubleSpinBox *servoSpinBoxes[2],
                               QDoubleSpinBox *muscleSpinBoxes[8],
                               QSlider *servoSliders[2],
                               QSlider *muscleSliders[8],
                               const QString &armName);
    void sendLeftArmCommand();
    void sendRightArmCommand();
    void sendStopCommand();
    void resetAllParameters();  // 将所有界面参数归零
    void logMessage(const QString &message);

    // 示教功能辅助函数
    Waypoint getCurrentWaypoint();              // 获取当前界面的角度值作为路径点(包含步进电机高度)
    void loadWaypointToUI(const Waypoint &wp);  // 将路径点加载到界面(包含步进电机高度)
    void saveTrajectoryToJSON(const QString &filename);  // 保存轨迹到JSON文件
    void loadTrajectoryFromJSON(const QString &filename); // 从JSON文件加载轨迹
    void saveTrajectoryToXML(const QString &filename);  // 保存轨迹到XML文件（保留用于兼容）
    void loadTrajectoryFromXML(const QString &filename); // 从XML文件加载轨迹（保留用于兼容）
    void executeWaypoint(const Waypoint &wp);   // 执行单个路径点(包含步进电机高度)
    void stopTrajectory();                      // 停止轨迹执行
    void sendStepperHeightCommand(double height); // 发送步进电机高度命令
};

#endif // STATICSCONTROL_H

