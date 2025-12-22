#include "staticscontrol.h"
#include "ui_staticscontrol.h"
#include <QThread>  // 用于添加延时

StaticsControl::StaticsControl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StaticsControl),
    waypointCounter(0),
    isTrajectoryRunning(false),
    currentTrajectoryIndex(0)
{
    ui->setupUi(this);
    setupUI();

    // 初始化轨迹执行定时器
    trajectoryTimer = new QTimer(this);
    connect(trajectoryTimer, &QTimer::timeout, this, &StaticsControl::executeNextTrajectoryStep);

    // 注意: 不需要手动连接按钮信号!
    // Qt 的自动连接机制会自动连接 on_<objectName>_<signalName> 格式的槽函数
    // 如果手动连接会导致信号被触发两次!

    logMessage("示教功能已初始化");
}

StaticsControl::~StaticsControl()
{
    delete ui;
}

void StaticsControl::setupUI()
{
    // 创建左臂控制组
    createArmControlGroup(ui->groupBox_left_arm,
                         leftServoSpinBoxes,
                         leftMuscleSpinBoxes,
                         leftServoSliders,
                         leftMuscleSliders,
                         "左臂");

    // 创建右臂控制组
    createArmControlGroup(ui->groupBox_right_arm,
                         rightServoSpinBoxes,
                         rightMuscleSpinBoxes,
                         rightServoSliders,
                         rightMuscleSliders,
                         "右臂");

    // 注意: 步进电机滑块和输入框的连接由Qt自动连接机制处理
    // 槽函数: on_horizontalSlider_stepper_height_valueChanged 和 on_doubleSpinBox_stepper_height_valueChanged
}

void StaticsControl::createArmControlGroup(QGroupBox *groupBox,
                                           QDoubleSpinBox *servoSpinBoxes[2],
                                           QDoubleSpinBox *muscleSpinBoxes[8],
                                           QSlider *servoSliders[2],
                                           QSlider *muscleSliders[8],
                                           const QString &armName)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(groupBox);
    
    // 伺服电机控制区域
    QGroupBox *servoGroup = new QGroupBox("伺服电机", groupBox);
    QGridLayout *servoLayout = new QGridLayout(servoGroup);
    
    for (int i = 0; i < 2; i++) {
        QLabel *label = new QLabel(QString("电机 %1:").arg(i + 1), servoGroup);
        servoSpinBoxes[i] = new QDoubleSpinBox(servoGroup);
        servoSpinBoxes[i]->setRange(-90.0, 90.0);
        servoSpinBoxes[i]->setValue(0.0);
        servoSpinBoxes[i]->setSuffix(" °");
        servoSpinBoxes[i]->setDecimals(2);
        servoSpinBoxes[i]->setSingleStep(1);
        servoSpinBoxes[i]->setMaximumWidth(70);  // 限制 spinbox 最大宽度

        servoSliders[i] = new QSlider(Qt::Horizontal, servoGroup);
        servoSliders[i]->setRange(-90, 90);
        servoSliders[i]->setValue(0);
        servoSliders[i]->setSingleStep(1);   // 键盘箭头每次移动 1
        servoSliders[i]->setPageStep(10);  // 鼠标点击空白区域每次跳跃 10

        // 连接滑块和数值框
        connect(servoSliders[i], &QSlider::valueChanged, this, [=](int value) {
            servoSpinBoxes[i]->setValue(value );
        });
        connect(servoSpinBoxes[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [=](double value) {
            servoSliders[i]->setValue(static_cast<int>(value));
        });

        servoLayout->addWidget(label, i, 0);
        servoLayout->addWidget(servoSpinBoxes[i], i, 1);
        servoLayout->addWidget(servoSliders[i], i, 2);
    }

    // 设置列拉伸比例: 标签不拉伸, spinbox 占1份, slider 占4份
    servoLayout->setColumnStretch(0, 0);  // 标签列
    servoLayout->setColumnStretch(1, 1);  // spinbox 列
    servoLayout->setColumnStretch(2, 4);  // slider 列

    mainLayout->addWidget(servoGroup);
    
    // 肌肉关节控制区域
    QGroupBox *muscleGroup = new QGroupBox("肌肉关节", groupBox);
    QGridLayout *muscleLayout = new QGridLayout(muscleGroup);

    for (int i = 0; i < 4; i++) {
        QLabel *jointLabel = new QLabel(QString("关节 %1:").arg(i + 1), muscleGroup);
        muscleLayout->addWidget(jointLabel, i * 2, 0, 2, 1);

        for (int j = 0; j < 2; j++) {
            int index = i * 2 + j;
            QLabel *angleLabel = new QLabel(QString("角度 %1:").arg(j + 1), muscleGroup);
            muscleSpinBoxes[index] = new QDoubleSpinBox(muscleGroup);
            muscleSpinBoxes[index]->setRange(-30.0, 30.0);
            muscleSpinBoxes[index]->setValue(0.0);
            muscleSpinBoxes[index]->setSuffix(" °");
            muscleSpinBoxes[index]->setDecimals(2);
            muscleSpinBoxes[index]->setSingleStep(1);
            muscleSpinBoxes[index]->setMaximumWidth(70);  // 限制 spinbox 最大宽度,让它更窄

            muscleSliders[index] = new QSlider(Qt::Horizontal, muscleGroup);
            muscleSliders[index]->setRange(-30, 30);
            muscleSliders[index]->setValue(0);
            muscleSliders[index]->setSingleStep(1);   // 键盘箭头每次移动 1
            muscleSliders[index]->setPageStep(5);  // 鼠标点击空白区域每次跳跃 5

            // 连接滑块和数值框
            connect(muscleSliders[index], &QSlider::valueChanged, this, [=](int value) {
                muscleSpinBoxes[index]->setValue(value );
            });
            connect(muscleSpinBoxes[index], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, [=](double value) {
                muscleSliders[index]->setValue(static_cast<int>(value ));
            });

            muscleLayout->addWidget(angleLabel, i * 2 + j, 1);
            muscleLayout->addWidget(muscleSpinBoxes[index], i * 2 + j, 2);
            muscleLayout->addWidget(muscleSliders[index], i * 2 + j, 3);
        }
    }

    // 设置列拉伸比例: 关节标签和角度标签不拉伸, spinbox 占1份, slider 占5份
    muscleLayout->setColumnStretch(0, 0);  // 关节标签列
    muscleLayout->setColumnStretch(1, 0);  // 角度标签列
    muscleLayout->setColumnStretch(2, 1);  // spinbox 列
    muscleLayout->setColumnStretch(3, 5);  // slider 列 (占更多空间)

    mainLayout->addWidget(muscleGroup);
}

void StaticsControl::on_pushButton_send_clicked()
{
    // 发送步进电机高度命令 (新增)
    double stepperHeight = ui->doubleSpinBox_stepper_height->value();
    sendStepperHeightCommand(stepperHeight);

    // 发送左右臂命令
    sendLeftArmCommand();
    sendRightArmCommand();

    logMessage(QString("发送全部控制命令 (高度: %1m)").arg(stepperHeight, 0, 'f', 3));
}

void StaticsControl::on_pushButton_send_left_clicked()
{
    sendLeftArmCommand();
    logMessage("发送左臂控制命令");
}

void StaticsControl::on_pushButton_send_right_clicked()
{
    sendRightArmCommand();
    logMessage("发送右臂控制命令");
}

void StaticsControl::on_pushButton_stop_clicked()
{
    sendStopCommand();
    resetAllParameters();  // 停止后将界面参数归零
    logMessage("紧急停止并归零界面");
}

void StaticsControl::on_pushButton_reset_clicked()
{
    resetAllParameters();  // 仅归零界面,不发送消息
    logMessage("界面参数已归零");
}

void StaticsControl::sendLeftArmCommand()
{
    // 获取全局信号发送器
    SignalTransmitter *signal_transmitter = SignalTransmitter::instance();

    // ========== 新架构: 使用 ArmTarget 消息 ==========
    ihb_msgs::msg::ArmTarget leftArmMsg;
    leftArmMsg.cmd = OPEN_LOOP_with_statics;  // 静力学模型开环控制

    // 准备2个伺服电机关节 (ID: 1, 2)
    // 注意: servo_joints 是固定大小的 std::array<ServoMotorJointTarget, 2>, 不需要 resize
    for (int i = 0; i < 2; i++) {
        leftArmMsg.servo_joints[i].id = i + 1;  // 左臂电机ID: 1, 2
        leftArmMsg.servo_joints[i].cmd = 3;  // SET_TARGET
        leftArmMsg.servo_joints[i].angle = static_cast<float>(leftServoSpinBoxes[i]->value() * 100);
        leftArmMsg.servo_joints[i].speed_limit = 1000;  // 默认速度限制

        qDebug() << "准备左臂伺服电机" << (i+1) << "角度:" << leftServoSpinBoxes[i]->value();
    }

    // 准备4个肌肉关节 (ID: 0, 1, 2, 3)
    // 注意: muscle_joints 是固定大小的 std::array<MuscleJointTarget, 4>, 不需要 resize
    for (int i = 0; i < 4; i++) {
        leftArmMsg.muscle_joints[i].id = i;  // 左臂肌肉关节ID: 0, 1, 2, 3
        leftArmMsg.muscle_joints[i].cmd = OPEN_LOOP_with_statics;  // 静力学模型开环控制

        // 设置两个角度参数
        leftArmMsg.muscle_joints[i].angle[0] = static_cast<float>(leftMuscleSpinBoxes[i * 2]->value());
        leftArmMsg.muscle_joints[i].angle[1] = static_cast<float>(leftMuscleSpinBoxes[i * 2 + 1]->value());

        // 压力值设置为默认值(静力学模型会根据角度计算)
        for (int j = 0; j < 4; j++) {
            leftArmMsg.muscle_joints[i].pressure[j] = 0;
        }

        qDebug() << "准备左臂肌肉关节" << i
                 << "角度1:" << leftMuscleSpinBoxes[i * 2]->value()
                 << "角度2:" << leftMuscleSpinBoxes[i * 2 + 1]->value();
    }

    // 发送左臂目标消息 (新架构: 单个消息包含所有关节)
    emit signal_transmitter->ihbLeftArmTargetSignal(leftArmMsg);

    qDebug() << "左臂命令已发送 (新架构: ArmTarget消息, cmd=" << leftArmMsg.cmd
             << ", 伺服电机:" << leftArmMsg.servo_joints.size()
             << "个, 肌肉关节:" << leftArmMsg.muscle_joints.size() << "个)";
}

void StaticsControl::sendRightArmCommand()
{
    // 获取全局信号发送器
    SignalTransmitter *signal_transmitter = SignalTransmitter::instance();

    // ========== 新架构: 使用 ArmTarget 消息 ==========
    ihb_msgs::msg::ArmTarget rightArmMsg;
    rightArmMsg.cmd = OPEN_LOOP_with_statics;  // 静力学模型开环控制

    // 准备2个伺服电机关节 (ID: 3, 4)
    // 注意: servo_joints 是固定大小的 std::array<ServoMotorJointTarget, 2>, 不需要 resize
    for (int i = 0; i < 2; i++) {
        rightArmMsg.servo_joints[i].id = i + 3;  // 右臂电机ID: 3, 4
        rightArmMsg.servo_joints[i].cmd = 3;  // SET_TARGET
        rightArmMsg.servo_joints[i].angle = static_cast<float>(rightServoSpinBoxes[i]->value() * 100);
        rightArmMsg.servo_joints[i].speed_limit = 300;  // 默认速度限制

        qDebug() << "准备右臂伺服电机" << (i+3) << "角度:" << rightServoSpinBoxes[i]->value();
    }

    // 准备4个肌肉关节 (ID: 4, 5, 6, 7)
    // 注意: muscle_joints 是固定大小的 std::array<MuscleJointTarget, 4>, 不需要 resize
    for (int i = 0; i < 4; i++) {
        rightArmMsg.muscle_joints[i].id = i + 4;  // 右臂肌肉关节ID: 4, 5, 6, 7
        rightArmMsg.muscle_joints[i].cmd = OPEN_LOOP_with_statics;  // 静力学模型开环控制

        // 设置两个角度参数
        rightArmMsg.muscle_joints[i].angle[0] = static_cast<float>(rightMuscleSpinBoxes[i * 2]->value());
        rightArmMsg.muscle_joints[i].angle[1] = static_cast<float>(rightMuscleSpinBoxes[i * 2 + 1]->value());

        // 压力值设置为默认值(静力学模型会根据角度计算)
        for (int j = 0; j < 4; j++) {
            rightArmMsg.muscle_joints[i].pressure[j] = 0;
        }

        qDebug() << "准备右臂肌肉关节" << (i+4)
                 << "角度1:" << rightMuscleSpinBoxes[i * 2]->value()
                 << "角度2:" << rightMuscleSpinBoxes[i * 2 + 1]->value();
    }

    // 发送右臂目标消息 (新架构: 单个消息包含所有关节)
    emit signal_transmitter->ihbRightArmTargetSignal(rightArmMsg);

    qDebug() << "右臂命令已发送 (新架构: ArmTarget消息, cmd=" << rightArmMsg.cmd
             << ", 伺服电机:" << rightArmMsg.servo_joints.size()
             << "个, 肌肉关节:" << rightArmMsg.muscle_joints.size() << "个)";
}

void StaticsControl::sendStopCommand()
{
    // 获取全局信号发送器
    SignalTransmitter *signal_transmitter = SignalTransmitter::instance();

    // ========== 新架构: 使用 ArmTarget 消息发送停止命令 ==========

    // 1. 发送左臂停止命令
    ihb_msgs::msg::ArmTarget leftArmStopMsg;
    leftArmStopMsg.cmd = OPEN_LOOP_with_statics;  // 使用静力学模型模式

    // 准备2个伺服电机关节 (ID: 1, 2) - 设置为零点位置
    for (int i = 0; i < 2; i++) {
        leftArmStopMsg.servo_joints[i].id = i + 1;  // 左臂电机ID: 1, 2
        leftArmStopMsg.servo_joints[i].cmd = 3;  // SET_TARGET
        leftArmStopMsg.servo_joints[i].angle = 0.0;  // 零点位置
        leftArmStopMsg.servo_joints[i].speed_limit = 300;  // 较低的速度限制以确保安全停止
    }

    // 准备4个肌肉关节 (ID: 0, 1, 2, 3) - 设置为零点位置
    for (int i = 0; i < 4; i++) {
        leftArmStopMsg.muscle_joints[i].id = i;  // 左臂肌肉关节ID: 0, 1, 2, 3
        leftArmStopMsg.muscle_joints[i].cmd = OPEN_LOOP_with_statics;

        // 设置角度为0
        leftArmStopMsg.muscle_joints[i].angle[0] = 0.0;
        leftArmStopMsg.muscle_joints[i].angle[1] = 0.0;

        // 压力值设置为0
        for (int j = 0; j < 4; j++) {
            leftArmStopMsg.muscle_joints[i].pressure[j] = 0;
        }
    }

    // 发送左臂停止命令
    emit signal_transmitter->ihbLeftArmTargetSignal(leftArmStopMsg);
    qDebug() << "左臂停止命令已发送 (新架构: ArmTarget消息, 所有关节归零)";

    // 2. 发送右臂停止命令
    ihb_msgs::msg::ArmTarget rightArmStopMsg;
    rightArmStopMsg.cmd = OPEN_LOOP_with_statics;  // 使用静力学模型模式

    // 准备2个伺服电机关节 (ID: 3, 4) - 设置为零点位置
    for (int i = 0; i < 2; i++) {
        rightArmStopMsg.servo_joints[i].id = i + 3;  // 右臂电机ID: 3, 4
        rightArmStopMsg.servo_joints[i].cmd = 3;  // SET_TARGET
        rightArmStopMsg.servo_joints[i].angle = 0.0;  // 零点位置
        rightArmStopMsg.servo_joints[i].speed_limit = 300;  // 较低的速度限制以确保安全停止
    }

    // 准备4个肌肉关节 (ID: 4, 5, 6, 7) - 设置为零点位置
    for (int i = 0; i < 4; i++) {
        rightArmStopMsg.muscle_joints[i].id = i + 4;  // 右臂肌肉关节ID: 4, 5, 6, 7
        rightArmStopMsg.muscle_joints[i].cmd = OPEN_LOOP_with_statics;

        // 设置角度为0
        rightArmStopMsg.muscle_joints[i].angle[0] = 0.0;
        rightArmStopMsg.muscle_joints[i].angle[1] = 0.0;

        // 压力值设置为0
        for (int j = 0; j < 4; j++) {
            rightArmStopMsg.muscle_joints[i].pressure[j] = 0;
        }
    }

    // 发送右臂停止命令
    emit signal_transmitter->ihbRightArmTargetSignal(rightArmStopMsg);
    qDebug() << "右臂停止命令已发送 (新架构: ArmTarget消息, 所有关节归零)";

    qDebug() << "所有电机和肌肉关节已停止 (新架构: 左右臂分别发送ArmTarget消息)";
}

void StaticsControl::resetAllParameters()
{
    // 将步进电机高度归零 (新增)
    ui->doubleSpinBox_stepper_height->setValue(0.0);

    // 将所有左臂参数归零
    for (int i = 0; i < 2; i++) {
        leftServoSpinBoxes[i]->setValue(0.0);
        leftServoSliders[i]->setValue(0);
    }

    for (int i = 0; i < 8; i++) {
        leftMuscleSpinBoxes[i]->setValue(0.0);
        leftMuscleSliders[i]->setValue(0);
    }

    // 将所有右臂参数归零
    for (int i = 0; i < 2; i++) {
        rightServoSpinBoxes[i]->setValue(0.0);
        rightServoSliders[i]->setValue(0);
    }

    for (int i = 0; i < 8; i++) {
        rightMuscleSpinBoxes[i]->setValue(0.0);
        rightMuscleSliders[i]->setValue(0);
    }

    qDebug() << "所有界面参数已归零 (包括步进电机高度)";
}

void StaticsControl::logMessage(const QString &message)
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString logText = QString("[%1] %2").arg(currentTime).arg(message);

    // 输出到控制台
    qDebug() << logText;

    // 如果UI中有日志显示区域，可以在这里添加
    if (ui->textEdit_log) {
        ui->textEdit_log->append(logText);
    }
}

// ==================== 示教功能实现 ====================

// 记录路径点按钮槽函数
void StaticsControl::on_pushButton_record_waypoint_clicked()
{
    Waypoint wp = getCurrentWaypoint();
    waypointCounter++;
    wp.name = QString("路径点 %1").arg(waypointCounter);

    waypointList.append(wp);

    // 添加到统一步骤列表
    TrajectoryStep step;
    step.type = STEP_WAYPOINT;
    step.waypoint = wp;
    trajectorySteps.append(step);

    // 添加到列表显示 (包含步进电机高度)
    QString displayText = QString("%1 (高度: %2m | 左伺服: %3°, %4° | 右伺服: %5°, %6°)")
        .arg(wp.name)
        .arg(wp.stepperHeight, 0, 'f', 3)
        .arg(wp.leftServoAngles[0], 0, 'f', 1)
        .arg(wp.leftServoAngles[1], 0, 'f', 1)
        .arg(wp.rightServoAngles[0], 0, 'f', 1)
        .arg(wp.rightServoAngles[1], 0, 'f', 1);

    ui->listWidget_waypoints->addItem(displayText);

    logMessage(QString("已记录路径点: %1 (高度: %2m)").arg(wp.name).arg(wp.stepperHeight, 0, 'f', 3));
}

// 删除路径点按钮槽函数
void StaticsControl::on_pushButton_delete_waypoint_clicked()
{
    int currentRow = ui->listWidget_waypoints->currentRow();
    if (currentRow >= 0 && currentRow < trajectorySteps.size()) {
        TrajectoryStep step = trajectorySteps[currentRow];
        QString name;

        if (step.type == STEP_WAYPOINT) {
            name = step.waypoint.name;
            // 同时从旧的waypointList中删除
            for (int i = 0; i < waypointList.size(); i++) {
                if (waypointList[i].name == name) {
                    waypointList.removeAt(i);
                    break;
                }
            }
        } else {
            name = QString("延时 %1 秒").arg(step.delaySeconds);
            // 同时从旧的delayList中删除
            for (int i = 0; i < delayList.size(); i++) {
                if (qAbs(delayList[i].delaySeconds - step.delaySeconds) < 0.001) {
                    delayList.removeAt(i);
                    break;
                }
            }
        }

        trajectorySteps.removeAt(currentRow);
        delete ui->listWidget_waypoints->takeItem(currentRow);
        logMessage(QString("已删除: %1").arg(name));
    } else {
        QMessageBox::warning(this, "警告", "请先选择要删除的路径点或延时!");
    }
}

// 插入延时按钮槽函数
void StaticsControl::on_pushButton_insert_delay_clicked()
{
    double delaySeconds = ui->doubleSpinBox_delay->value();
    DelayCommand delay(delaySeconds);

    // 创建延时步骤
    TrajectoryStep step;
    step.type = STEP_DELAY;
    step.delaySeconds = delaySeconds;

    int currentRow = ui->listWidget_waypoints->currentRow();
    if (currentRow >= 0) {
        delayList.insert(currentRow + 1, delay);
        trajectorySteps.insert(currentRow + 1, step);
        ui->listWidget_waypoints->insertItem(currentRow + 1, delay.name);
        logMessage(QString("已插入延时: %1 秒").arg(delaySeconds));
    } else {
        delayList.append(delay);
        trajectorySteps.append(step);
        ui->listWidget_waypoints->addItem(delay.name);
        logMessage(QString("已添加延时: %1 秒").arg(delaySeconds));
    }
}

// 保存轨迹按钮槽函数
void StaticsControl::on_pushButton_save_trajectory_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "保存轨迹文件", "", "JSON文件 (*.json);;XML文件 (*.xml)");
    if (!fileName.isEmpty()) {
        if (fileName.endsWith(".json", Qt::CaseInsensitive)) {
            saveTrajectoryToJSON(fileName);
        } else {
            saveTrajectoryToXML(fileName);
        }
        logMessage(QString("轨迹已保存到: %1").arg(fileName));
    }
}

// 加载轨迹按钮槽函数
void StaticsControl::on_pushButton_load_trajectory_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "加载轨迹文件", "", "JSON文件 (*.json);;XML文件 (*.xml)");
    if (!fileName.isEmpty()) {
        if (fileName.endsWith(".json", Qt::CaseInsensitive)) {
            loadTrajectoryFromJSON(fileName);
        } else {
            loadTrajectoryFromXML(fileName);
        }
        logMessage(QString("轨迹已从文件加载: %1").arg(fileName));
    }
}

// 运行/停止轨迹按钮槽函数
void StaticsControl::on_pushButton_run_trajectory_clicked()
{
    if (!isTrajectoryRunning) {
        // 开始运行轨迹
        if (ui->listWidget_waypoints->count() == 0) {
            QMessageBox::warning(this, "警告", "轨迹列表为空,请先添加路径点!");
            return;
        }

        isTrajectoryRunning = true;
        currentTrajectoryIndex = 0;
        ui->pushButton_run_trajectory->setText("⏸️ 停止轨迹");
        ui->pushButton_run_trajectory->setStyleSheet("background-color: #e74c3c; font-size: 16px;");

        logMessage("开始执行轨迹...");
        executeNextTrajectoryStep();
    } else {
        // 停止轨迹
        stopTrajectory();
    }
}

// 清空轨迹按钮槽函数
void StaticsControl::on_pushButton_clear_trajectory_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认", "确定要清空所有路径点吗?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        waypointList.clear();
        delayList.clear();
        trajectorySteps.clear();
        ui->listWidget_waypoints->clear();
        waypointCounter = 0;
        logMessage("轨迹已清空");
    }
}

// 双击路径点加载到界面
void StaticsControl::on_listWidget_waypoints_itemDoubleClicked(QListWidgetItem *item)
{
    int row = ui->listWidget_waypoints->row(item);

    // 检查是否是延时指令
    if (item->text().contains("等待")) {
        logMessage("这是延时指令,无法加载到界面");
        return;
    }

    // 查找对应的路径点
    int waypointIndex = 0;
    for (int i = 0; i <= row; i++) {
        if (!ui->listWidget_waypoints->item(i)->text().contains("等待")) {
            if (i == row) {
                break;
            }
            waypointIndex++;
        }
    }

    if (waypointIndex < waypointList.size()) {
        loadWaypointToUI(waypointList[waypointIndex]);
        logMessage(QString("已加载路径点到界面: %1").arg(waypointList[waypointIndex].name));
    }
}

// 单击路径点加载到界面
void StaticsControl::on_listWidget_waypoints_itemClicked(QListWidgetItem *item)
{
    int row = ui->listWidget_waypoints->row(item);

    // 检查是否是延时指令
    if (item->text().contains("等待")) {
        logMessage("这是延时指令,无法加载到界面");
        return;
    }

    // 查找对应的路径点
    int waypointIndex = 0;
    for (int i = 0; i <= row; i++) {
        if (!ui->listWidget_waypoints->item(i)->text().contains("等待")) {
            if (i == row) {
                break;
            }
            waypointIndex++;
        }
    }

    if (waypointIndex < waypointList.size()) {
        loadWaypointToUI(waypointList[waypointIndex]);
        logMessage(QString("已加载路径点到界面: %1").arg(waypointList[waypointIndex].name));
    }
}

// ==================== 示教功能辅助函数 ====================

// 获取当前界面的角度值作为路径点
Waypoint StaticsControl::getCurrentWaypoint()
{
    Waypoint wp;

    // 获取步进电机高度 (新增)
    wp.stepperHeight = ui->doubleSpinBox_stepper_height->value();

    // 获取左臂伺服电机角度
    for (int i = 0; i < 2; i++) {
        wp.leftServoAngles[i] = leftServoSpinBoxes[i]->value();
    }

    // 获取左臂肌肉关节角度
    for (int i = 0; i < 8; i++) {
        wp.leftMuscleAngles[i] = leftMuscleSpinBoxes[i]->value();
    }

    // 获取右臂伺服电机角度
    for (int i = 0; i < 2; i++) {
        wp.rightServoAngles[i] = rightServoSpinBoxes[i]->value();
    }

    // 获取右臂肌肉关节角度
    for (int i = 0; i < 8; i++) {
        wp.rightMuscleAngles[i] = rightMuscleSpinBoxes[i]->value();
    }

    return wp;
}

// 将路径点加载到界面
void StaticsControl::loadWaypointToUI(const Waypoint &wp)
{
    // 加载步进电机高度 (新增)
    ui->doubleSpinBox_stepper_height->setValue(wp.stepperHeight);

    // 加载左臂伺服电机角度
    for (int i = 0; i < 2; i++) {
        leftServoSpinBoxes[i]->setValue(wp.leftServoAngles[i]);
    }

    // 加载左臂肌肉关节角度
    for (int i = 0; i < 8; i++) {
        leftMuscleSpinBoxes[i]->setValue(wp.leftMuscleAngles[i]);
    }

    // 加载右臂伺服电机角度
    for (int i = 0; i < 2; i++) {
        rightServoSpinBoxes[i]->setValue(wp.rightServoAngles[i]);
    }

    // 加载右臂肌肉关节角度
    for (int i = 0; i < 8; i++) {
        rightMuscleSpinBoxes[i]->setValue(wp.rightMuscleAngles[i]);
    }
}

// 执行单个路径点
void StaticsControl::executeWaypoint(const Waypoint &wp)
{
    // 先加载到界面
    loadWaypointToUI(wp);

    // 发送步进电机高度命令 (新增)
    sendStepperHeightCommand(wp.stepperHeight);

    // 然后发送左右臂命令
    sendLeftArmCommand();
    sendRightArmCommand();

    logMessage(QString("正在执行路径点: %1 (高度: %2m)").arg(wp.name).arg(wp.stepperHeight, 0, 'f', 3));
}

// 执行下一个轨迹步骤
void StaticsControl::executeNextTrajectoryStep()
{
    if (!isTrajectoryRunning) {
        return;
    }
    if (currentTrajectoryIndex >= ui->listWidget_waypoints->count()) {
        // 轨迹执行完成
        
        stopTrajectory();
        logMessage("轨迹执行完成!");
        return;
    }

    QListWidgetItem *item = ui->listWidget_waypoints->item(currentTrajectoryIndex);
    QString itemText = item->text();

    // 高亮当前执行的项
    ui->listWidget_waypoints->setCurrentRow(currentTrajectoryIndex);

    if (itemText.contains("等待")) {
        // 这是延时指令
        QRegExp regExp("等待\\s*(\\d+(\\.\\d+)?)\\s*秒");
        if (regExp.indexIn(itemText) != -1) {
            double delaySeconds = regExp.cap(1).toDouble();
            logMessage(QString("延时 %1 秒...").arg(delaySeconds));

            // 使用定时器实现延时
            trajectoryTimer->start(static_cast<int>(delaySeconds * 1000));
            currentTrajectoryIndex++;
            return;
        }
    } else {
        // 这是路径点,查找对应的Waypoint
        int waypointIndex = 0;
        for (int i = 0; i < currentTrajectoryIndex; i++) {
            if (!ui->listWidget_waypoints->item(i)->text().contains("等待")) {
                waypointIndex++;
            }
        }

        if (waypointIndex < waypointList.size()) {
            executeWaypoint(waypointList[waypointIndex]);
        }

        // 继续下一步(给机器人一点执行时间)
        trajectoryTimer->start(500);  // 500ms后执行下一步
        currentTrajectoryIndex++;
    }
}

// 停止轨迹执行
void StaticsControl::stopTrajectory()
{
    isTrajectoryRunning = false;
    trajectoryTimer->stop();
    currentTrajectoryIndex = 0;

    ui->pushButton_run_trajectory->setText("▶️ 运行轨迹");
    ui->pushButton_run_trajectory->setStyleSheet("background-color: #27ae60; font-size: 16px;");

    // 注意: 不再自动发送停止命令和归零
    // 用户如果需要归零，可以手动点击归零按钮，或者在轨迹数据中明确包含归零步骤

    logMessage("轨迹执行已停止");
}

// 保存轨迹到JSON文件
void StaticsControl::saveTrajectoryToJSON(const QString &filename)
{
    QJsonObject root;
    root["trajectoryName"] = "轨迹文件";
    root["version"] = "1.0";

    QJsonArray stepsArray;

    // 遍历统一的步骤列表
    for (const TrajectoryStep &step : trajectorySteps) {
        QJsonObject stepObj;

        if (step.type == STEP_WAYPOINT) {
            // 保存路径点
            stepObj["type"] = "waypoint";
            stepObj["name"] = step.waypoint.name;

            QJsonObject dataObj;
            dataObj["stepperHeight"] = step.waypoint.stepperHeight;

            // 左侧舵机角度
            QJsonArray leftServoArray;
            for (int i = 0; i < 2; i++) {
                leftServoArray.append(step.waypoint.leftServoAngles[i]);
            }
            dataObj["leftServoAngles"] = leftServoArray;

            // 左侧肌肉角度
            QJsonArray leftMuscleArray;
            for (int i = 0; i < 8; i++) {
                leftMuscleArray.append(step.waypoint.leftMuscleAngles[i]);
            }
            dataObj["leftMuscleAngles"] = leftMuscleArray;

            // 右侧舵机角度
            QJsonArray rightServoArray;
            for (int i = 0; i < 2; i++) {
                rightServoArray.append(step.waypoint.rightServoAngles[i]);
            }
            dataObj["rightServoAngles"] = rightServoArray;

            // 右侧肌肉角度
            QJsonArray rightMuscleArray;
            for (int i = 0; i < 8; i++) {
                rightMuscleArray.append(step.waypoint.rightMuscleAngles[i]);
            }
            dataObj["rightMuscleAngles"] = rightMuscleArray;

            stepObj["data"] = dataObj;
        } else {
            // 保存延时
            stepObj["type"] = "delay";
            stepObj["duration"] = step.delaySeconds;
        }

        stepsArray.append(stepObj);
    }

    root["steps"] = stepsArray;

    // 保存到文件
    QJsonDocument doc(root);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << doc.toJson(QJsonDocument::Indented);
        file.close();
    } else {
        QMessageBox::critical(this, "错误", "无法保存文件: " + filename);
    }
}

// 保存轨迹到XML文件
void StaticsControl::saveTrajectoryToXML(const QString &filename)
{
    QDomDocument doc("Trajectory");
    QDomElement root = doc.createElement("TrajectoryData");
    doc.appendChild(root);

    // 遍历列表中的所有项
    for (int i = 0; i < ui->listWidget_waypoints->count(); i++) {
        QListWidgetItem *item = ui->listWidget_waypoints->item(i);
        QString itemText = item->text();

        if (itemText.contains("等待")) {
            // 保存延时指令
            QDomElement delayElement = doc.createElement("Delay");

            QRegExp regExp("等待\\s*(\\d+(\\.\\d+)?)\\s*秒");
            if (regExp.indexIn(itemText) != -1) {
                double delaySeconds = regExp.cap(1).toDouble();
                delayElement.setAttribute("seconds", delaySeconds);
            }

            root.appendChild(delayElement);
        } else {
            // 保存路径点
            // 查找对应的Waypoint索引
            int waypointIndex = 0;
            for (int j = 0; j < i; j++) {
                if (!ui->listWidget_waypoints->item(j)->text().contains("等待")) {
                    waypointIndex++;
                }
            }

            if (waypointIndex < waypointList.size()) {
                const Waypoint &wp = waypointList[waypointIndex];
                QDomElement wpElement = doc.createElement("Waypoint");
                wpElement.setAttribute("name", wp.name);

                // 保存步进电机高度 (新增)
                QDomElement stepperElem = doc.createElement("StepperHeight");
                stepperElem.appendChild(doc.createTextNode(QString::number(wp.stepperHeight)));
                wpElement.appendChild(stepperElem);

                // 保存左臂伺服电机角度
                for (int j = 0; j < 2; j++) {
                    QDomElement elem = doc.createElement(QString("LeftServo_%1").arg(j));
                    elem.appendChild(doc.createTextNode(QString::number(wp.leftServoAngles[j])));
                    wpElement.appendChild(elem);
                }

                // 保存左臂肌肉关节角度
                for (int j = 0; j < 8; j++) {
                    QDomElement elem = doc.createElement(QString("LeftMuscle_%1").arg(j));
                    elem.appendChild(doc.createTextNode(QString::number(wp.leftMuscleAngles[j])));
                    wpElement.appendChild(elem);
                }

                // 保存右臂伺服电机角度
                for (int j = 0; j < 2; j++) {
                    QDomElement elem = doc.createElement(QString("RightServo_%1").arg(j));
                    elem.appendChild(doc.createTextNode(QString::number(wp.rightServoAngles[j])));
                    wpElement.appendChild(elem);
                }

                // 保存右臂肌肉关节角度
                for (int j = 0; j < 8; j++) {
                    QDomElement elem = doc.createElement(QString("RightMuscle_%1").arg(j));
                    elem.appendChild(doc.createTextNode(QString::number(wp.rightMuscleAngles[j])));
                    wpElement.appendChild(elem);
                }

                root.appendChild(wpElement);
            }
        }
    }

    // 保存到文件
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << doc.toString(4);  // 4个空格缩进
        file.close();
    } else {
        QMessageBox::critical(this, "错误", "无法保存文件: " + filename);
    }
}

// 从JSON文件加载轨迹
void StaticsControl::loadTrajectoryFromJSON(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件: " + filename);
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::critical(this, "错误", "无法解析JSON文件");
        return;
    }

    // 清空当前轨迹
    waypointList.clear();
    delayList.clear();
    trajectorySteps.clear();
    ui->listWidget_waypoints->clear();
    waypointCounter = 0;

    QJsonObject root = doc.object();
    QJsonArray stepsArray = root["steps"].toArray();

    for (const QJsonValue &stepValue : stepsArray) {
        QJsonObject stepObj = stepValue.toObject();
        QString type = stepObj["type"].toString();

        TrajectoryStep step;

        if (type == "waypoint") {
            // 加载路径点
            step.type = STEP_WAYPOINT;
            Waypoint wp;
            wp.name = stepObj["name"].toString();

            QJsonObject dataObj = stepObj["data"].toObject();
            wp.stepperHeight = dataObj["stepperHeight"].toDouble();

            // 左侧舵机角度
            QJsonArray leftServoArray = dataObj["leftServoAngles"].toArray();
            for (int i = 0; i < 2 && i < leftServoArray.size(); i++) {
                wp.leftServoAngles[i] = leftServoArray[i].toDouble();
            }

            // 左侧肌肉角度
            QJsonArray leftMuscleArray = dataObj["leftMuscleAngles"].toArray();
            for (int i = 0; i < 8 && i < leftMuscleArray.size(); i++) {
                wp.leftMuscleAngles[i] = leftMuscleArray[i].toDouble();
            }

            // 右侧舵机角度
            QJsonArray rightServoArray = dataObj["rightServoAngles"].toArray();
            for (int i = 0; i < 2 && i < rightServoArray.size(); i++) {
                wp.rightServoAngles[i] = rightServoArray[i].toDouble();
            }

            // 右侧肌肉角度
            QJsonArray rightMuscleArray = dataObj["rightMuscleAngles"].toArray();
            for (int i = 0; i < 8 && i < rightMuscleArray.size(); i++) {
                wp.rightMuscleAngles[i] = rightMuscleArray[i].toDouble();
            }

            step.waypoint = wp;
            waypointList.append(wp);
            waypointCounter++;

            // 添加到列表显示
            QString displayText = QString("%1 (高度: %2m | 左伺服: %3°, %4° | 右伺服: %5°, %6°)")
                .arg(wp.name)
                .arg(wp.stepperHeight, 0, 'f', 3)
                .arg(wp.leftServoAngles[0], 0, 'f', 1)
                .arg(wp.leftServoAngles[1], 0, 'f', 1)
                .arg(wp.rightServoAngles[0], 0, 'f', 1)
                .arg(wp.rightServoAngles[1], 0, 'f', 1);

            ui->listWidget_waypoints->addItem(displayText);
        } else if (type == "delay") {
            // 加载延时
            step.type = STEP_DELAY;
            step.delaySeconds = stepObj["duration"].toDouble();

            DelayCommand delay(step.delaySeconds);
            delayList.append(delay);
            ui->listWidget_waypoints->addItem(delay.name);
        }

        trajectorySteps.append(step);
    }
}

// 从XML文件加载轨迹
void StaticsControl::loadTrajectoryFromXML(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件: " + filename);
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        QMessageBox::critical(this, "错误", "无法解析XML文件");
        return;
    }
    file.close();

    // 清空当前轨迹
    waypointList.clear();
    delayList.clear();
    trajectorySteps.clear();
    ui->listWidget_waypoints->clear();
    waypointCounter = 0;

    QDomElement root = doc.documentElement();
    QDomNodeList children = root.childNodes();

    for (int i = 0; i < children.count(); i++) {
        QDomElement element = children.at(i).toElement();

        TrajectoryStep step;

        if (element.tagName() == "Delay") {
            // 加载延时指令
            double delaySeconds = element.attribute("seconds").toDouble();
            DelayCommand delay(delaySeconds);
            delayList.append(delay);
            ui->listWidget_waypoints->addItem(delay.name);

            step.type = STEP_DELAY;
            step.delaySeconds = delaySeconds;
            trajectorySteps.append(step);
        } else if (element.tagName() == "Waypoint") {
            // 加载路径点
            Waypoint wp;
            wp.name = element.attribute("name");

            // 加载步进电机高度 (新增)
            QDomElement stepperElem = element.firstChildElement("StepperHeight");
            if (!stepperElem.isNull()) {
                wp.stepperHeight = stepperElem.text().toDouble();
            }

            // 加载左臂伺服电机角度
            for (int j = 0; j < 2; j++) {
                QDomElement elem = element.firstChildElement(QString("LeftServo_%1").arg(j));
                if (!elem.isNull()) {
                    wp.leftServoAngles[j] = elem.text().toDouble();
                }
            }

            // 加载左臂肌肉关节角度
            for (int j = 0; j < 8; j++) {
                QDomElement elem = element.firstChildElement(QString("LeftMuscle_%1").arg(j));
                if (!elem.isNull()) {
                    wp.leftMuscleAngles[j] = elem.text().toDouble();
                }
            }

            // 加载右臂伺服电机角度
            for (int j = 0; j < 2; j++) {
                QDomElement elem = element.firstChildElement(QString("RightServo_%1").arg(j));
                if (!elem.isNull()) {
                    wp.rightServoAngles[j] = elem.text().toDouble();
                }
            }

            // 加载右臂肌肉关节角度
            for (int j = 0; j < 8; j++) {
                QDomElement elem = element.firstChildElement(QString("RightMuscle_%1").arg(j));
                if (!elem.isNull()) {
                    wp.rightMuscleAngles[j] = elem.text().toDouble();
                }
            }

            waypointList.append(wp);
            waypointCounter++;

            step.type = STEP_WAYPOINT;
            step.waypoint = wp;
            trajectorySteps.append(step);

            // 添加到列表显示 (包含步进电机高度)
            QString displayText = QString("%1 (高度: %2m | 左伺服: %3°, %4° | 右伺服: %5°, %6°)")
                .arg(wp.name)
                .arg(wp.stepperHeight, 0, 'f', 3)
                .arg(wp.leftServoAngles[0], 0, 'f', 1)
                .arg(wp.leftServoAngles[1], 0, 'f', 1)
                .arg(wp.rightServoAngles[0], 0, 'f', 1)
                .arg(wp.rightServoAngles[1], 0, 'f', 1);

            ui->listWidget_waypoints->addItem(displayText);
        }
    }
}

// 步进电机滑块值改变槽函数
void StaticsControl::on_horizontalSlider_stepper_height_valueChanged(int value)
{
    // 同步到输入框 (阻塞信号避免循环触发)
    ui->doubleSpinBox_stepper_height->blockSignals(true);
    ui->doubleSpinBox_stepper_height->setValue(value / 1000.0);
    ui->doubleSpinBox_stepper_height->blockSignals(false);

    // 发送步进电机命令
    double height = value / 1000.0;
    // sendStepperHeightCommand(height);
    // logMessage(QString("步进电机高度: %1 m").arg(height, 0, 'f', 3));
}

// 步进电机输入框值改变槽函数
void StaticsControl::on_doubleSpinBox_stepper_height_valueChanged(double value)
{
    // 同步到滑块 (阻塞信号避免循环触发)
    ui->horizontalSlider_stepper_height->blockSignals(true);
    ui->horizontalSlider_stepper_height->setValue(static_cast<int>(value * 1000));
    ui->horizontalSlider_stepper_height->blockSignals(false);

    // 发送步进电机命令
    // sendStepperHeightCommand(value);
    // logMessage(QString("步进电机高度: %1 m").arg(value, 0, 'f', 3));
}

// 发送步进电机高度命令
void StaticsControl::sendStepperHeightCommand(double height)
{
    // 获取全局信号发送器
    SignalTransmitter *signal_transmitter = SignalTransmitter::instance();

    // 创建步进电机消息
    ihb_msgs::msg::StepperMotorJointTarget stepperTarget;
    stepperTarget.id = 0;  // 步进电机ID
    stepperTarget.height = static_cast<float>(height);

    // 发送信号
    emit signal_transmitter->ihbStepperTargetChangedSignal(stepperTarget);
}

