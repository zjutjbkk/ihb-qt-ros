#ifndef PIDDEBUGWIDGET_H
#define PIDDEBUGWIDGET_H

#include <QWidget>
#include <QTime>
#include <QVector>
#include <QMutex>
#include "pidjointwidget.h"
#include "pidparammanager.h"
#include "src/connect/signaltransmitter.h"
#include "ihb_msgs/msg/ihb_status.hpp"
#include "ihb_msgs/msg/muscle_joint_target.hpp"
#include "ihb_msgs/msg/arm_target.hpp"
#include "qcustomplot.h"

// 控制模式宏定义
#define CLOSE_PID 0  // 关闭PID控制
#define OPEN_LOOP_with_statics 1  // 静力学模型开环控制
#define PID_CONTROL 2  // PID控制

namespace Ui {
class PIDDebugWidget;
}

class PIDDebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PIDDebugWidget(QWidget *parent = nullptr);
    ~PIDDebugWidget();

private slots:
    // 状态更新
    void onIhbStatusUpdate(const ihb_msgs::msg::IhbStatus &msg);

    // 参数操作
    void onJointParamsChanged(int jointId, double kp, double ki, double kd);
    void onSendSingleJoint(int jointId);
    void onSendAllJoints();
    void onStopPidControl();  // 新增：停止PID控制

    // 配置管理
    void onSaveConfig();
    void onLoadConfig();
    void onConfigSelected(const QString &configName);
    void onResetToDefaults();

    // 曲线控制
    void onPausePlot();
    void onClearPlot();

private:
    Ui::PIDDebugWidget *ui;

    // 8个关节组件
    PIDJointWidget *jointWidgets[8];

    // 参数管理器
    PidParamManager *paramManager;

    // 信号发送器
    SignalTransmitter *signalTransmitter;

    // 曲线绘制相关
    QTime plotStartTime;
    bool isPlotPaused;
    int maxDataPoints;  // 最大数据点数量
    QMutex plotMutex;   // 线程安全保护

    // 时间数据
    QVector<double> timeData;

    // 角度数据 (16条曲线：8个muscle × 2个angle)
    // angleData[i] 对应 muscle i 的 angle[0] 或 angle[1]
    // 索引 0-7: muscle 0-7 的 angle[0]
    // 索引 8-15: muscle 0-7 的 angle[1]
    QVector<QVector<double>> angleData;  // 16条曲线数据

    // 曲线可见性控制
    QVector<bool> curveVisible;  // 16个布尔值，控制每条曲线是否显示

    // 辅助函数
    void setupUI();
    void sendPidParams(int jointId, double kp, double ki, double kd);
    void loadConfigList();

    // 曲线绘制函数
    void setupAnglePlot();
    void updateAnglePlot(const ihb_msgs::msg::IhbStatus &msg);
    void clearPlotData();
};

#endif // PIDDEBUGWIDGET_H

