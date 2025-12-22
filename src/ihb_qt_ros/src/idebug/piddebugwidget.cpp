#include "piddebugwidget.h"
#include "ui_piddebugwidget.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QMutexLocker>

PIDDebugWidget::PIDDebugWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PIDDebugWidget),
    isPlotPaused(false),
    maxDataPoints(1000)  // 最多保留1000个数据点
{
    ui->setupUi(this);

    // 获取单例
    paramManager = PidParamManager::instance();
    signalTransmitter = SignalTransmitter::instance();

    // 初始化曲线数据存储 (16条曲线)
    angleData.resize(16);
    curveVisible.resize(16);
    for (int i = 0; i < 16; i++) {
        angleData[i].reserve(maxDataPoints);
        curveVisible[i] = true;  // 默认所有曲线可见
    }
    timeData.reserve(maxDataPoints);

    // 初始化8个关节组件，使用网格布局（2列4行）
    for (int i = 0; i < 8; i++) {
        jointWidgets[i] = new PIDJointWidget(i, this);

        // 计算网格位置：2列布局
        int row = i / 2;     // 行号：0, 0, 1, 1, 2, 2, 3, 3
        int col = i % 2;     // 列号：0, 1, 0, 1, 0, 1, 0, 1

        ui->gridLayout_joints->addWidget(jointWidgets[i], row, col);

        // 连接信号
        connect(jointWidgets[i], &PIDJointWidget::paramsChanged,
                this, &PIDDebugWidget::onJointParamsChanged);
        connect(jointWidgets[i], &PIDJointWidget::sendParamsClicked,
                this, &PIDDebugWidget::onSendSingleJoint);
    }

    // 连接 ihb_status 信号
    connect(signalTransmitter, &SignalTransmitter::IhbStatusChangedSignal,
            this, &PIDDebugWidget::onIhbStatusUpdate, Qt::QueuedConnection);

    // 连接配置管理按钮
    connect(ui->pushButton_save_config, &QPushButton::clicked, this, &PIDDebugWidget::onSaveConfig);
    connect(ui->pushButton_load_config, &QPushButton::clicked, this, &PIDDebugWidget::onLoadConfig);
    connect(ui->comboBox_config, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
            this, &PIDDebugWidget::onConfigSelected);
    connect(ui->pushButton_reset, &QPushButton::clicked, this, &PIDDebugWidget::onResetToDefaults);

    // 连接批量操作按钮
    connect(ui->pushButton_send_all, &QPushButton::clicked, this, &PIDDebugWidget::onSendAllJoints);
    connect(ui->pushButton_stop, &QPushButton::clicked, this, &PIDDebugWidget::onStopPidControl);

    // 连接曲线控制按钮
    connect(ui->pushButton_pause_plot, &QPushButton::clicked, this, &PIDDebugWidget::onPausePlot);
    connect(ui->pushButton_clear_plot, &QPushButton::clicked, this, &PIDDebugWidget::onClearPlot);

    // 初始化曲线
    setupAnglePlot();

    // 初始化时间
    plotStartTime = QTime::currentTime();

    // 加载配置列表
    loadConfigList();

    // 加载默认参数到界面
    onResetToDefaults();
}

PIDDebugWidget::~PIDDebugWidget()
{
    delete ui;
}

void PIDDebugWidget::setupUI()
{
    // UI已在构造函数中设置
}

void PIDDebugWidget::onIhbStatusUpdate(const ihb_msgs::msg::IhbStatus &msg)
{
    // 更新8个关节组件的状态显示
    for (int i = 0; i < 8; i++) {
        jointWidgets[i]->updateStatus(msg.muscle_joint[i]);
    }

    // 更新曲线（如果未暂停）
    if (!isPlotPaused) {
        updateAnglePlot(msg);
    }
}

void PIDDebugWidget::sendPidParams(int jointId, double kp, double ki, double kd)
{
    // 创建 MuscleJointTarget 消息
    ihb_msgs::msg::MuscleJointTarget msg;
    msg.id = jointId;
    msg.cmd = 2;  // PID_CONTROL 模式

    // 设置 PID 参数
    msg.kp = static_cast<float>(kp);
    msg.ki = static_cast<float>(ki);
    msg.kd = static_cast<float>(kd);

    // 获取目标角度（从对应的 jointWidget 获取）
    double targetAngle0, targetAngle1;
    jointWidgets[jointId]->getTargetAngles(targetAngle0, targetAngle1);
    msg.angle[0] = static_cast<float>(targetAngle0);
    msg.angle[1] = static_cast<float>(targetAngle1);

    // 压力和刚度设置为默认值（PID模式下不使用）
    for (int i = 0; i < 4; i++) {
        msg.pressure[i] = 19999;
    }
    msg.stiffness[0] = 255;
    msg.stiffness[1] = 255;

    // 通过 SignalTransmitter 发送
    emit signalTransmitter->ihbMuscleTargetChangedSignal(msg);

    qDebug() << "发送PID参数到关节" << jointId
             << "Kp:" << kp << "Ki:" << ki << "Kd:" << kd
             << "目标角度: [" << targetAngle0 << "," << targetAngle1 << "]";
}

void PIDDebugWidget::onJointParamsChanged(int jointId, double kp, double ki, double kd)
{
    // 保存到参数管理器
    paramManager->setJointParams(jointId, kp, ki, kd);
}

void PIDDebugWidget::onSendSingleJoint(int jointId)
{
    double kp, ki, kd;
    jointWidgets[jointId]->getParams(kp, ki, kd);
    sendPidParams(jointId, kp, ki, kd);
    
    QMessageBox::information(this, "发送成功", 
        QString("已发送关节%1的PID参数\nKp: %2, Ki: %3, Kd: %4")
        .arg(jointId).arg(kp).arg(ki).arg(kd));
}

void PIDDebugWidget::onSendAllJoints()
{
    for (int i = 0; i < 8; i++) {
        double kp, ki, kd;
        jointWidgets[i]->getParams(kp, ki, kd);
        sendPidParams(i, kp, ki, kd);
    }
    qDebug() << "已发送所有关节的PID参数";
    QMessageBox::information(this, "发送成功", "已发送所有8个关节的PID参数");
}

void PIDDebugWidget::onSaveConfig()
{
    bool ok;
    QString configName = QInputDialog::getText(this, "保存配置",
                                               "请输入配置名称:",
                                               QLineEdit::Normal,
                                               "新配置", &ok);
    if (ok && !configName.isEmpty()) {
        if (paramManager->saveConfig(configName)) {
            QMessageBox::information(this, "成功", "配置已保存");
            loadConfigList();
        } else {
            QMessageBox::warning(this, "错误", "配置保存失败");
        }
    }
}

void PIDDebugWidget::onLoadConfig()
{
    QString configName = ui->comboBox_config->currentText();
    if (configName.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先选择一个配置");
        return;
    }

    if (paramManager->loadConfig(configName)) {
        PidConfig config = paramManager->getCurrentConfig();
        for (int i = 0; i < 8; i++) {
            double kp, ki, kd;
            paramManager->getJointParams(i, kp, ki, kd);
            jointWidgets[i]->setParams(kp, ki, kd);
        }
        qDebug() << "已加载配置:" << configName;
        QMessageBox::information(this, "成功", QString("已加载配置: %1").arg(configName));
    } else {
        QMessageBox::warning(this, "错误", "配置加载失败");
    }
}

void PIDDebugWidget::onConfigSelected(const QString &configName)
{
    // 配置选择变更时自动加载
    if (!configName.isEmpty()) {
        onLoadConfig();
    }
}

void PIDDebugWidget::onResetToDefaults()
{
    paramManager->resetToDefaults();
    for (int i = 0; i < 8; i++) {
        double kp, ki, kd;
        PidParamManager::getDefaultParams(i, kp, ki, kd);
        jointWidgets[i]->setParams(kp, ki, kd);
    }
    qDebug() << "已恢复默认PID参数";
    QMessageBox::information(this, "成功", "已恢复所有关节的默认PID参数");
}

void PIDDebugWidget::loadConfigList()
{
    ui->comboBox_config->clear();
    QStringList configs = paramManager->getConfigList();
    if (!configs.isEmpty()) {
        ui->comboBox_config->addItems(configs);
    }
}

void PIDDebugWidget::onPausePlot()
{
    isPlotPaused = ui->pushButton_pause_plot->isChecked();
    if (isPlotPaused) {
        ui->pushButton_pause_plot->setText("▶️ 继续");
        qDebug() << "曲线绘制已暂停";
    } else {
        ui->pushButton_pause_plot->setText("⏸️ 暂停");
        qDebug() << "曲线绘制已继续";
    }
}

void PIDDebugWidget::onClearPlot()
{
    clearPlotData();
    qDebug() << "曲线数据已清空";
}

void PIDDebugWidget::setupAnglePlot()
{
    QCustomPlot *plot = ui->customPlot_angle;
    if (!plot) {
        qDebug() << "角度曲线控件未找到!";
        return;
    }

    plot->clearGraphs();
    plot->legend->setVisible(true);
    plot->legend->setFont(QFont("Helvetica", 8));
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // 连接图例点击信号，实现曲线显示/隐藏切换
    connect(plot, &QCustomPlot::legendClick, this, [this, plot](QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event) {
        Q_UNUSED(legend);
        Q_UNUSED(event);

        // 获取被点击的图例项对应的曲线
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        if (plItem) {
            QCPGraph *graph = qobject_cast<QCPGraph*>(plItem->plottable());
            if (graph) {
                // 找到对应的曲线索引
                int graphIndex = -1;
                for (int i = 0; i < plot->graphCount(); i++) {
                    if (plot->graph(i) == graph) {
                        graphIndex = i;
                        break;
                    }
                }

                if (graphIndex >= 0 && graphIndex < 16) {
                    // 切换可见性
                    curveVisible[graphIndex] = !curveVisible[graphIndex];
                    graph->setVisible(curveVisible[graphIndex]);

                    // 更新图例项的透明度
                    if (curveVisible[graphIndex]) {
                        plItem->setTextColor(Qt::black);
                    } else {
                        plItem->setTextColor(QColor(150, 150, 150));  // 灰色表示隐藏
                    }

                    plot->replot();
                    qDebug() << "曲线" << graphIndex << (curveVisible[graphIndex] ? "显示" : "隐藏");
                }
            }
        }
    });

    // 定义颜色数组（8种不同颜色）
    QColor colors[8] = {
        QColor(255, 0, 0),      // 红色
        QColor(0, 0, 255),      // 蓝色
        QColor(0, 200, 0),      // 绿色
        QColor(255, 165, 0),    // 橙色
        QColor(255, 0, 255),    // 紫色
        QColor(0, 200, 200),    // 青色
        QColor(139, 69, 19),    // 棕色
        QColor(128, 128, 128)   // 灰色
    };

    // 创建16条曲线：8个muscle × 2个angle
    // 索引 0-7: muscle 0-7 的 angle[0]（实线）
    // 索引 8-15: muscle 0-7 的 angle[1]（虚线）

    // 先创建所有16条曲线
    for (int i = 0; i < 16; i++) {
        plot->addGraph();
    }

    // 然后设置每条曲线的属性
    for (int i = 0; i < 8; i++) {
        // muscle i 的 angle[0] 曲线（实线）
        plot->graph(i)->setPen(QPen(colors[i], 2));
        plot->graph(i)->setName(QString("M%1-A[0]").arg(i));

        // muscle i 的 angle[1] 曲线（虚线）
        QPen dashedPen(colors[i], 1.5, Qt::DashLine);
        plot->graph(i + 8)->setPen(dashedPen);
        plot->graph(i + 8)->setName(QString("M%1-A[1]").arg(i));
    }

    // 设置坐标轴
    plot->xAxis->setLabel("时间 (秒)");
    plot->yAxis->setLabel("角度 (度)");
    plot->xAxis->setRange(0, 10);
    plot->yAxis->setRange(-180, 180);

    // 设置网格
    plot->xAxis->grid()->setVisible(true);
    plot->yAxis->grid()->setVisible(true);
    plot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
    plot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));

    plot->replot();
}

void PIDDebugWidget::updateAnglePlot(const ihb_msgs::msg::IhbStatus &msg)
{
    QMutexLocker locker(&plotMutex);

    QCustomPlot *plot = ui->customPlot_angle;
    if (!plot) return;

    // 计算当前时间（相对于启动时间的秒数）
    double currentTime = plotStartTime.msecsTo(QTime::currentTime()) / 1000.0;

    // 添加时间数据点
    timeData.append(currentTime);

    // 添加16条曲线的数据点
    // 索引 0-7: muscle 0-7 的 angle[0]
    // 索引 8-15: muscle 0-7 的 angle[1]
    for (int i = 0; i < 8; i++) {
        angleData[i].append(msg.muscle_joint[i].angle[0]);      // angle[0]
        angleData[i + 8].append(msg.muscle_joint[i].angle[1]);  // angle[1]
    }

    // 限制数据点数量
    if (timeData.size() > maxDataPoints) {
        timeData.removeFirst();
        for (int i = 0; i < 16; i++) {
            if (angleData[i].size() > maxDataPoints) {
                angleData[i].removeFirst();
            }
        }
    }

    // 更新图表数据（16条曲线）
    for (int i = 0; i < 16; i++) {
        if (curveVisible[i]) {
            plot->graph(i)->setData(timeData, angleData[i]);
        }
    }

    // 自动调整x轴范围
    double maxTime = currentTime;
    double minTime = (maxTime > 10) ? (maxTime - 10) : 0;
    plot->xAxis->setRange(minTime, maxTime);

    // 重新绘制
    plot->replot();
}

void PIDDebugWidget::clearPlotData()
{
    QMutexLocker locker(&plotMutex);

    // 清空所有数据
    timeData.clear();
    for (int i = 0; i < 16; i++) {
        angleData[i].clear();
    }

    // 重置开始时间
    plotStartTime = QTime::currentTime();

    // 清空图表
    QCustomPlot *anglePlot = ui->customPlot_angle;
    if (anglePlot) {
        for (int i = 0; i < anglePlot->graphCount(); i++) {
            anglePlot->graph(i)->data()->clear();
        }
        anglePlot->replot();
    }
}

void PIDDebugWidget::onStopPidControl()
{
    // ========== 正确的停止方式: 发送 CLOSE_PID 命令 ==========
    // 下位机收到 cmd=0 后会:
    // 1. 停止PID控制线程
    // 2. 归零所有气压输出
    // 3. 归零伺服电机位置

    // 发送左臂停止命令
    ihb_msgs::msg::ArmTarget leftArmStopMsg;
    leftArmStopMsg.cmd = CLOSE_PID;  // cmd = 0: 关闭PID控制
    emit signalTransmitter->ihbLeftArmTargetSignal(leftArmStopMsg);

    // 发送右臂停止命令
    ihb_msgs::msg::ArmTarget rightArmStopMsg;
    rightArmStopMsg.cmd = CLOSE_PID;  // cmd = 0: 关闭PID控制
    emit signalTransmitter->ihbRightArmTargetSignal(rightArmStopMsg);

    qDebug() << "已发送停止PID控制命令 (cmd=CLOSE_PID)";
    QMessageBox::information(this, "停止PID控制",
        "已发送停止命令到下位机\n"
        "下位机将:\n"
        "1. 停止PID控制线程\n"
        "2. 归零所有气压输出\n"
        "3. 归零伺服电机位置");
}

