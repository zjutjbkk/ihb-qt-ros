// teach.cpp
#include "teach.h"
#include "ui_teach.h"
#include "src/connect/signaltransmitter.h"

teach::teach(QWidget *parent) :
    //在构造函数中初始化成员变量
    QWidget(parent),
    ui(new Ui::teach),
    waypointCount(0),           // 初始化 waypoint 计数器
    process(new QProcess(this)),// 创建 QProcess 对象
    //运行按钮相关变量
        isRunning(false),           // 用于运行按钮判断状态
        currentWaypointIndex(0),    // waypoint索引用于运行按钮判断当前 waypoint的行数
        // isWaypointReached(false),   // 判断waypoint是否到达 
        // waypointTimer(new QTimer(this))//isWaypointReached超时计时器
        //运行按钮线程
        workerThread(nullptr),
        worker_ptr(nullptr)

{
    //appendToTextEdit(outputWithTime);运行时会有奇怪的报错：
    //     QObject::connect: Cannot queue arguments of type 'QTextCursor'
    // (Make sure 'QTextCursor' is registered using qRegisterMetaType().)
    qRegisterMetaType<QTextCursor>("QTextCursor");

    ui->setupUi(this);
    // 1.连接"添加点"按钮
        connect(ui->pushButton_def, &QPushButton::clicked, this, [=]() {
            waypointCount++;
            QString point = QString("waypoint%1").arg(waypointCount);
            QListWidgetItem *item = new QListWidgetItem(point);

            // 将所有 LineEdit 中的数据组合成字符串，存储到 UserRole
            QString data = QString(
                "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,"
                "%21,%22,%23,%24,%25,%26,%27,%28,%29,%30,%31,%32,%33,%34,%35,%36,%37,%38,%39,%40,"
                "%41,%42,%43,%44"
            ).arg(ui->LineEdit_motor_left_1_id->text())
             .arg(ui->LineEdit_motor_left_1_position->text())
             .arg(ui->LineEdit_motor_left_1_velocity->text())
             .arg(ui->LineEdit_motor_left_2_id->text())
             .arg(ui->LineEdit_motor_left_2_position->text())
             .arg(ui->LineEdit_motor_left_2_velocity->text())
             .arg(ui->LineEdit_motor_right_1_id->text())
             .arg(ui->LineEdit_motor_right_1_position->text())
             .arg(ui->LineEdit_motor_right_1_velocity->text())
             .arg(ui->LineEdit_motor_right_2_id->text())
             .arg(ui->LineEdit_motor_right_2_position->text())
             .arg(ui->LineEdit_motor_right_2_velocity->text())

             // 添加机械臂左侧的数据
             .arg(ui->LineEdit_arm_left_j1_m1->text())
             .arg(ui->LineEdit_arm_left_j1_m2->text())
             .arg(ui->LineEdit_arm_left_j1_m3->text())
             .arg(ui->LineEdit_arm_left_j1_m4->text())
             .arg(ui->LineEdit_arm_left_j2_m1->text())
             .arg(ui->LineEdit_arm_left_j2_m2->text())
             .arg(ui->LineEdit_arm_left_j2_m3->text())
             .arg(ui->LineEdit_arm_left_j2_m4->text())
             .arg(ui->LineEdit_arm_left_j3_m1->text())
             .arg(ui->LineEdit_arm_left_j3_m2->text())
             .arg(ui->LineEdit_arm_left_j3_m3->text())
             .arg(ui->LineEdit_arm_left_j3_m4->text())
             .arg(ui->LineEdit_arm_left_j4_m1->text())
             .arg(ui->LineEdit_arm_left_j4_m2->text())
             .arg(ui->LineEdit_arm_left_j4_m3->text())
             .arg(ui->LineEdit_arm_left_j4_m4->text())

             // 添加机械臂右侧的数据
             .arg(ui->LineEdit_arm_right_j1_m1->text())
             .arg(ui->LineEdit_arm_right_j1_m2->text())
             .arg(ui->LineEdit_arm_right_j1_m3->text())
             .arg(ui->LineEdit_arm_right_j1_m4->text())
             .arg(ui->LineEdit_arm_right_j2_m1->text())
             .arg(ui->LineEdit_arm_right_j2_m2->text())
             .arg(ui->LineEdit_arm_right_j2_m3->text())
             .arg(ui->LineEdit_arm_right_j2_m4->text())
             .arg(ui->LineEdit_arm_right_j3_m1->text())
             .arg(ui->LineEdit_arm_right_j3_m2->text())
             .arg(ui->LineEdit_arm_right_j3_m3->text())
             .arg(ui->LineEdit_arm_right_j3_m4->text())
             .arg(ui->LineEdit_arm_right_j4_m1->text())
             .arg(ui->LineEdit_arm_right_j4_m2->text())
             .arg(ui->LineEdit_arm_right_j4_m3->text())
             .arg(ui->LineEdit_arm_right_j4_m4->text());

            // 将数据存储到 UserRole 中
            item->setData(Qt::UserRole, data);

            // 将新创建的列表项添加到 listWidget 中
            ui->listWidget->addItem(item);
        });

        // 2.连接 listWidget 的 itemClicked 信号
        connect(ui->listWidget, &QListWidget::itemClicked, this, [=](QListWidgetItem *item) {
            QStringList values = item->data(Qt::UserRole).toString().split(",");

            // 确保有完整的 44 个数据项
            if (values.size() == 44) {
                // 设置电机左侧的值
                ui->LineEdit_motor_left_1_id->setText(values[0]);
                ui->LineEdit_motor_left_1_position->setText(values[1]);
                ui->LineEdit_motor_left_1_velocity->setText(values[2]);

                ui->LineEdit_motor_left_2_id->setText(values[3]);
                ui->LineEdit_motor_left_2_position->setText(values[4]);
                ui->LineEdit_motor_left_2_velocity->setText(values[5]);

                // 设置电机右侧的值
                ui->LineEdit_motor_right_1_id->setText(values[6]);
                ui->LineEdit_motor_right_1_position->setText(values[7]);
                ui->LineEdit_motor_right_1_velocity->setText(values[8]);

                ui->LineEdit_motor_right_2_id->setText(values[9]);
                ui->LineEdit_motor_right_2_position->setText(values[10]);
                ui->LineEdit_motor_right_2_velocity->setText(values[11]);

                // 设置机械臂左侧的值
                ui->LineEdit_arm_left_j1_m1->setText(values[12]);
                ui->LineEdit_arm_left_j1_m2->setText(values[13]);
                ui->LineEdit_arm_left_j1_m3->setText(values[14]);
                ui->LineEdit_arm_left_j1_m4->setText(values[15]);

                ui->LineEdit_arm_left_j2_m1->setText(values[16]);
                ui->LineEdit_arm_left_j2_m2->setText(values[17]);
                ui->LineEdit_arm_left_j2_m3->setText(values[18]);
                ui->LineEdit_arm_left_j2_m4->setText(values[19]);

                ui->LineEdit_arm_left_j3_m1->setText(values[20]);
                ui->LineEdit_arm_left_j3_m2->setText(values[21]);
                ui->LineEdit_arm_left_j3_m3->setText(values[22]);
                ui->LineEdit_arm_left_j3_m4->setText(values[23]);

                ui->LineEdit_arm_left_j4_m1->setText(values[24]);
                ui->LineEdit_arm_left_j4_m2->setText(values[25]);
                ui->LineEdit_arm_left_j4_m3->setText(values[26]);
                ui->LineEdit_arm_left_j4_m4->setText(values[27]);

                // 设置机械臂右侧的值
                ui->LineEdit_arm_right_j1_m1->setText(values[28]);
                ui->LineEdit_arm_right_j1_m2->setText(values[29]);
                ui->LineEdit_arm_right_j1_m3->setText(values[30]);
                ui->LineEdit_arm_right_j1_m4->setText(values[31]);

                ui->LineEdit_arm_right_j2_m1->setText(values[32]);
                ui->LineEdit_arm_right_j2_m2->setText(values[33]);
                ui->LineEdit_arm_right_j2_m3->setText(values[34]);
                ui->LineEdit_arm_right_j2_m4->setText(values[35]);

                ui->LineEdit_arm_right_j3_m1->setText(values[36]);
                ui->LineEdit_arm_right_j3_m2->setText(values[37]);
                ui->LineEdit_arm_right_j3_m3->setText(values[38]);
                ui->LineEdit_arm_right_j3_m4->setText(values[39]);

                ui->LineEdit_arm_right_j4_m1->setText(values[40]);
                ui->LineEdit_arm_right_j4_m2->setText(values[41]);
                ui->LineEdit_arm_right_j4_m3->setText(values[42]);
                ui->LineEdit_arm_right_j4_m4->setText(values[43]);
            } else {
                // 如果数据不完整，显示一个错误信息
                QMessageBox::warning(this, "错误", "数据不完整，无法显示");
            }
        });

        // 3.连接"向前添加点"按钮
        connect(ui->pushButton_add_forw, &QPushButton::clicked, this, [=]() {
            int currentRow = ui->listWidget->currentRow();
            if (currentRow != -1) {
                waypointCount++;
                QString point = QString("waypoint%1").arg(waypointCount);
                QListWidgetItem *item = new QListWidgetItem(point);

                // 获取所有 LineEdit 中的数据，组合成字符串并存储到 UserRole
                QString data = QString(
                    "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,"
                    "%21,%22,%23,%24,%25,%26,%27,%28,%29,%30,%31,%32,%33,%34,%35,%36,%37,%38,%39,%40,"
                    "%41,%42,%43,%44"
                ).arg(ui->LineEdit_motor_left_1_id->text())
                 .arg(ui->LineEdit_motor_left_1_position->text())
                 .arg(ui->LineEdit_motor_left_1_velocity->text())

                 .arg(ui->LineEdit_motor_left_2_id->text())
                 .arg(ui->LineEdit_motor_left_2_position->text())
                 .arg(ui->LineEdit_motor_left_2_velocity->text())

                 .arg(ui->LineEdit_motor_right_1_id->text())
                 .arg(ui->LineEdit_motor_right_1_position->text())
                 .arg(ui->LineEdit_motor_right_1_velocity->text())

                 .arg(ui->LineEdit_motor_right_2_id->text())
                 .arg(ui->LineEdit_motor_right_2_position->text())
                 .arg(ui->LineEdit_motor_right_2_velocity->text())


                 // 添加机械臂左侧的数据
                 .arg(ui->LineEdit_arm_left_j1_m1->text())
                 .arg(ui->LineEdit_arm_left_j1_m2->text())
                 .arg(ui->LineEdit_arm_left_j1_m3->text())
                 .arg(ui->LineEdit_arm_left_j1_m4->text())
                 .arg(ui->LineEdit_arm_left_j2_m1->text())
                 .arg(ui->LineEdit_arm_left_j2_m2->text())
                 .arg(ui->LineEdit_arm_left_j2_m3->text())
                 .arg(ui->LineEdit_arm_left_j2_m4->text())
                 .arg(ui->LineEdit_arm_left_j3_m1->text())
                 .arg(ui->LineEdit_arm_left_j3_m2->text())
                 .arg(ui->LineEdit_arm_left_j3_m3->text())
                 .arg(ui->LineEdit_arm_left_j3_m4->text())
                 .arg(ui->LineEdit_arm_left_j4_m1->text())
                 .arg(ui->LineEdit_arm_left_j4_m2->text())
                 .arg(ui->LineEdit_arm_left_j4_m3->text())
                 .arg(ui->LineEdit_arm_left_j4_m4->text())

                 // 添加机械臂右侧的数据
                 .arg(ui->LineEdit_arm_right_j1_m1->text())
                 .arg(ui->LineEdit_arm_right_j1_m2->text())
                 .arg(ui->LineEdit_arm_right_j1_m3->text())
                 .arg(ui->LineEdit_arm_right_j1_m4->text())
                 .arg(ui->LineEdit_arm_right_j2_m1->text())
                 .arg(ui->LineEdit_arm_right_j2_m2->text())
                 .arg(ui->LineEdit_arm_right_j2_m3->text())
                 .arg(ui->LineEdit_arm_right_j2_m4->text())
                 .arg(ui->LineEdit_arm_right_j3_m1->text())
                 .arg(ui->LineEdit_arm_right_j3_m2->text())
                 .arg(ui->LineEdit_arm_right_j3_m3->text())
                 .arg(ui->LineEdit_arm_right_j3_m4->text())
                 .arg(ui->LineEdit_arm_right_j4_m1->text())
                 .arg(ui->LineEdit_arm_right_j4_m2->text())
                 .arg(ui->LineEdit_arm_right_j4_m3->text())
                 .arg(ui->LineEdit_arm_right_j4_m4->text());

                // 将数据存储到 UserRole 中
                item->setData(Qt::UserRole, data);

                // 在当前行之前插入新项
                ui->listWidget->insertItem(currentRow, item);
            }
        });

        // 4.连接"向后添加点"按钮
        connect(ui->pushButton_add_back, &QPushButton::clicked, this, [=]() {
            int currentRow = ui->listWidget->currentRow();
            if (currentRow != -1) {
                waypointCount++;
                QString point = QString("waypoint%1").arg(waypointCount);
                QListWidgetItem *item = new QListWidgetItem(point);

                // 获取所有 LineEdit 中的数据，组合成字符串并存储到 UserRole
                QString data = QString(
                    "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,"
                    "%21,%22,%23,%24,%25,%26,%27,%28,%29,%30,%31,%32,%33,%34,%35,%36,%37,%38,%39,%40,"
                    "%41,%42,%43,%44"
                ).arg(ui->LineEdit_motor_left_1_id->text())
                 .arg(ui->LineEdit_motor_left_1_position->text())
                 .arg(ui->LineEdit_motor_left_1_velocity->text())

                 .arg(ui->LineEdit_motor_left_2_id->text())
                 .arg(ui->LineEdit_motor_left_2_position->text())
                 .arg(ui->LineEdit_motor_left_2_velocity->text())

                 .arg(ui->LineEdit_motor_right_1_id->text())
                 .arg(ui->LineEdit_motor_right_1_position->text())
                 .arg(ui->LineEdit_motor_right_1_velocity->text())

                 .arg(ui->LineEdit_motor_right_2_id->text())
                 .arg(ui->LineEdit_motor_right_2_position->text())
                 .arg(ui->LineEdit_motor_right_2_velocity->text())


                 // 添加机械臂左侧的数据
                 .arg(ui->LineEdit_arm_left_j1_m1->text())
                 .arg(ui->LineEdit_arm_left_j1_m2->text())
                 .arg(ui->LineEdit_arm_left_j1_m3->text())
                 .arg(ui->LineEdit_arm_left_j1_m4->text())
                 .arg(ui->LineEdit_arm_left_j2_m1->text())
                 .arg(ui->LineEdit_arm_left_j2_m2->text())
                 .arg(ui->LineEdit_arm_left_j2_m3->text())
                 .arg(ui->LineEdit_arm_left_j2_m4->text())
                 .arg(ui->LineEdit_arm_left_j3_m1->text())
                 .arg(ui->LineEdit_arm_left_j3_m2->text())
                 .arg(ui->LineEdit_arm_left_j3_m3->text())
                 .arg(ui->LineEdit_arm_left_j3_m4->text())
                 .arg(ui->LineEdit_arm_left_j4_m1->text())
                 .arg(ui->LineEdit_arm_left_j4_m2->text())
                 .arg(ui->LineEdit_arm_left_j4_m3->text())
                 .arg(ui->LineEdit_arm_left_j4_m4->text())

                 // 添加机械臂右侧的数据
                 .arg(ui->LineEdit_arm_right_j1_m1->text())
                 .arg(ui->LineEdit_arm_right_j1_m2->text())
                 .arg(ui->LineEdit_arm_right_j1_m3->text())
                 .arg(ui->LineEdit_arm_right_j1_m4->text())
                 .arg(ui->LineEdit_arm_right_j2_m1->text())
                 .arg(ui->LineEdit_arm_right_j2_m2->text())
                 .arg(ui->LineEdit_arm_right_j2_m3->text())
                 .arg(ui->LineEdit_arm_right_j2_m4->text())
                 .arg(ui->LineEdit_arm_right_j3_m1->text())
                 .arg(ui->LineEdit_arm_right_j3_m2->text())
                 .arg(ui->LineEdit_arm_right_j3_m3->text())
                 .arg(ui->LineEdit_arm_right_j3_m4->text())
                 .arg(ui->LineEdit_arm_right_j4_m1->text())
                 .arg(ui->LineEdit_arm_right_j4_m2->text())
                 .arg(ui->LineEdit_arm_right_j4_m3->text())
                 .arg(ui->LineEdit_arm_right_j4_m4->text());

                // 将数据存储到 UserRole 中
                item->setData(Qt::UserRole, data);

                // 在当前行之后插入新项
                ui->listWidget->insertItem(currentRow + 1, item);
            }
        });

        // 5.连接"删除选中点"按钮
        connect(ui->pushButton_del, &QPushButton::clicked, this, [=]() {
            int currentRow = ui->listWidget->currentRow();
            if (currentRow != -1) {
                delete ui->listWidget->takeItem(currentRow);
            }
        });

        // 6.连接"添加延时"按钮
        connect(ui->pushButton_add_delay, &QPushButton::clicked, this, [=]() {
            int currentRow = ui->listWidget->currentRow();
            if (currentRow != -1) {
                QString delay = "等待 " + QString::number(ui->doubleSpinBox->value()) + " 秒";
                QListWidgetItem *item = new QListWidgetItem(delay);
                item->setData(Qt::UserRole, delay);  // 将延时信息存储到UserRole中
                ui->listWidget->insertItem(currentRow + 1, item);
            }
        });


        // 7.连接"保存路径"按钮
        connect(ui->pushButton_save, &QPushButton::clicked, this, [=]() {
            QString fileName = QFileDialog::getSaveFileName(this, "保存路径", "", "XML文件 (*.xml)");
            if (!fileName.isEmpty()) {
                QDomDocument doc("Waypoints");
                QDomElement root = doc.createElement("WaypointsList");
                doc.appendChild(root);

                // 遍历所有的途径点
                for (int i = 0; i < ui->listWidget->count(); ++i) {
                    QListWidgetItem *item = ui->listWidget->item(i);

                    // 创建 waypoint 元素
                    QDomElement waypoint = doc.createElement("Waypoint");
                    waypoint.setAttribute("name", item->text());  // 设置waypoint名称

                    // 获取数据并拆分成 44 个元素
                    QStringList values = item->data(Qt::UserRole).toString().split(",");

                    // 如果是常规的途径点数据（非延时）
                    if (values.size() == 44) {
                        // 保存电机左侧的参数
                        QDomElement motor_left_1_id = doc.createElement("Motor_Left_1_ID");
                        motor_left_1_id.appendChild(doc.createTextNode(values[0]));
                        waypoint.appendChild(motor_left_1_id);

                        QDomElement motor_left_1_position = doc.createElement("Motor_Left_1_Position");
                        motor_left_1_position.appendChild(doc.createTextNode(values[1]));
                        waypoint.appendChild(motor_left_1_position);

                        QDomElement motor_left_1_velocity = doc.createElement("Motor_Left_1_Velocity");
                        motor_left_1_velocity.appendChild(doc.createTextNode(values[2]));
                        waypoint.appendChild(motor_left_1_velocity);

                        // 保存电机左侧的第二组参数
                        QDomElement motor_left_2_id = doc.createElement("Motor_Left_2_ID");
                        motor_left_2_id.appendChild(doc.createTextNode(values[3]));
                        waypoint.appendChild(motor_left_2_id);

                        QDomElement motor_left_2_position = doc.createElement("Motor_Left_2_Position");
                        motor_left_2_position.appendChild(doc.createTextNode(values[4]));
                        waypoint.appendChild(motor_left_2_position);

                        QDomElement motor_left_2_velocity = doc.createElement("Motor_Left_2_Velocity");
                        motor_left_2_velocity.appendChild(doc.createTextNode(values[5]));
                        waypoint.appendChild(motor_left_2_velocity);

                        // 保存电机右侧的参数
                        QDomElement motor_right_1_id = doc.createElement("Motor_Right_1_ID");
                        motor_right_1_id.appendChild(doc.createTextNode(values[6]));
                        waypoint.appendChild(motor_right_1_id);

                        QDomElement motor_right_1_position = doc.createElement("Motor_Right_1_Position");
                        motor_right_1_position.appendChild(doc.createTextNode(values[7]));
                        waypoint.appendChild(motor_right_1_position);

                        QDomElement motor_right_1_velocity = doc.createElement("Motor_Right_1_Velocity");
                        motor_right_1_velocity.appendChild(doc.createTextNode(values[8]));
                        waypoint.appendChild(motor_right_1_velocity);

                        QDomElement motor_right_2_id = doc.createElement("Motor_Right_2_ID");
                        motor_right_2_id.appendChild(doc.createTextNode(values[9]));
                        waypoint.appendChild(motor_right_2_id);

                        QDomElement motor_right_2_position = doc.createElement("Motor_Right_2_Position");
                        motor_right_2_position.appendChild(doc.createTextNode(values[10]));
                        waypoint.appendChild(motor_right_2_position);

                        QDomElement motor_right_2_velocity = doc.createElement("Motor_Right_2_Velocity");
                        motor_right_2_velocity.appendChild(doc.createTextNode(values[11]));
                        waypoint.appendChild(motor_right_2_velocity);

                        // 保存机械臂左侧的参数

                        // 创建并添加每个参数元素（分别为 m1,m2,m3,m4）
                        QDomElement arm_left_j1_m1 = doc.createElement("arm_left_j1_m1");
                        arm_left_j1_m1.appendChild(doc.createTextNode(values[12]));
                        waypoint.appendChild(arm_left_j1_m1);
                        QDomElement arm_left_j1_m2 = doc.createElement("arm_left_j1_m2");
                        arm_left_j1_m2.appendChild(doc.createTextNode(values[13]));
                        waypoint.appendChild(arm_left_j1_m2);
                        QDomElement arm_left_j1_m3 = doc.createElement("arm_left_j1_m3");
                        arm_left_j1_m3.appendChild(doc.createTextNode(values[14]));
                        waypoint.appendChild(arm_left_j1_m3);
                        QDomElement arm_left_j1_m4 = doc.createElement("arm_left_j1_m4");
                        arm_left_j1_m4.appendChild(doc.createTextNode(values[15]));
                        waypoint.appendChild(arm_left_j1_m4);

                        QDomElement arm_left_j2_m1 = doc.createElement("arm_left_j2_m1");
                        arm_left_j2_m1.appendChild(doc.createTextNode(values[16]));
                        waypoint.appendChild(arm_left_j2_m1);
                        QDomElement arm_left_j2_m2 = doc.createElement("arm_left_j2_m2");
                        arm_left_j2_m2.appendChild(doc.createTextNode(values[17]));
                        waypoint.appendChild(arm_left_j2_m2);
                        QDomElement arm_left_j2_m3 = doc.createElement("arm_left_j2_m3");
                        arm_left_j2_m3.appendChild(doc.createTextNode(values[18]));
                        waypoint.appendChild(arm_left_j2_m3);
                        QDomElement arm_left_j2_m4 = doc.createElement("arm_left_j2_m4");
                        arm_left_j2_m4.appendChild(doc.createTextNode(values[19]));
                        waypoint.appendChild(arm_left_j2_m4);

                        QDomElement arm_left_j3_m1 = doc.createElement("arm_left_j3_m1");
                        arm_left_j3_m1.appendChild(doc.createTextNode(values[20]));
                        waypoint.appendChild(arm_left_j3_m1);
                        QDomElement arm_left_j3_m2 = doc.createElement("arm_left_j3_m2");
                        arm_left_j3_m2.appendChild(doc.createTextNode(values[21]));
                        waypoint.appendChild(arm_left_j3_m2);
                        QDomElement arm_left_j3_m3 = doc.createElement("arm_left_j3_m3");
                        arm_left_j3_m3.appendChild(doc.createTextNode(values[22]));
                        waypoint.appendChild(arm_left_j3_m3);
                        QDomElement arm_left_j3_m4 = doc.createElement("arm_left_j3_m4");
                        arm_left_j3_m4.appendChild(doc.createTextNode(values[23]));
                        waypoint.appendChild(arm_left_j3_m4);


                        QDomElement arm_left_j4_m1 = doc.createElement("arm_left_j4_m1");
                        arm_left_j4_m1.appendChild(doc.createTextNode(values[24]));
                        waypoint.appendChild(arm_left_j4_m1);
                        QDomElement arm_left_j4_m2 = doc.createElement("arm_left_j4_m2");
                        arm_left_j4_m2.appendChild(doc.createTextNode(values[25]));
                        waypoint.appendChild(arm_left_j4_m2);
                        QDomElement arm_left_j4_m3 = doc.createElement("arm_left_j4_m3");
                        arm_left_j4_m3.appendChild(doc.createTextNode(values[26]));
                        waypoint.appendChild(arm_left_j4_m3);
                        QDomElement arm_left_j4_m4 = doc.createElement("arm_left_j4_m4");
                        arm_left_j4_m4.appendChild(doc.createTextNode(values[27]));
                        waypoint.appendChild(arm_left_j4_m4);
                        // 保存机械臂右侧的参数
                        QDomElement arm_right_j1_m1 = doc.createElement("arm_right_j1_m1");
                        arm_right_j1_m1.appendChild(doc.createTextNode(values[28]));
                        waypoint.appendChild(arm_right_j1_m1);
                        QDomElement arm_right_j1_m2 = doc.createElement("arm_right_j1_m2");
                        arm_right_j1_m2.appendChild(doc.createTextNode(values[29]));
                        waypoint.appendChild(arm_right_j1_m2);
                        QDomElement arm_right_j1_m3 = doc.createElement("arm_right_j1_m3");
                        arm_right_j1_m3.appendChild(doc.createTextNode(values[30]));
                        waypoint.appendChild(arm_right_j1_m3);
                        QDomElement arm_right_j1_m4 = doc.createElement("arm_right_j1_m4");
                        arm_right_j1_m4.appendChild(doc.createTextNode(values[31]));
                        waypoint.appendChild(arm_right_j1_m4);

                        QDomElement arm_right_j2_m1 = doc.createElement("arm_right_j2_m1");
                        arm_right_j2_m1.appendChild(doc.createTextNode(values[32]));
                        waypoint.appendChild(arm_right_j2_m1);
                        QDomElement arm_right_j2_m2 = doc.createElement("arm_right_j2_m2");
                        arm_right_j2_m2.appendChild(doc.createTextNode(values[33]));
                        waypoint.appendChild(arm_right_j2_m2);
                        QDomElement arm_right_j2_m3 = doc.createElement("arm_right_j2_m3");
                        arm_right_j2_m3.appendChild(doc.createTextNode(values[34]));
                        waypoint.appendChild(arm_right_j2_m3);
                        QDomElement arm_right_j2_m4 = doc.createElement("arm_right_j2_m4");
                        arm_right_j2_m4.appendChild(doc.createTextNode(values[35]));
                        waypoint.appendChild(arm_right_j2_m4);

                        QDomElement arm_right_j3_m1 = doc.createElement("arm_right_j3_m1");
                        arm_right_j3_m1.appendChild(doc.createTextNode(values[36]));
                        waypoint.appendChild(arm_right_j3_m1);
                        QDomElement arm_right_j3_m2 = doc.createElement("arm_right_j3_m2");
                        arm_right_j3_m2.appendChild(doc.createTextNode(values[37]));
                        waypoint.appendChild(arm_right_j3_m2);
                        QDomElement arm_right_j3_m3 = doc.createElement("arm_right_j3_m3");
                        arm_right_j3_m3.appendChild(doc.createTextNode(values[38]));
                        waypoint.appendChild(arm_right_j3_m3);
                        QDomElement arm_right_j3_m4 = doc.createElement("arm_right_j3_m4");
                        arm_right_j3_m4.appendChild(doc.createTextNode(values[39]));
                        waypoint.appendChild(arm_right_j3_m4);

                        QDomElement arm_right_j4_m1 = doc.createElement("arm_right_j4_m1");
                        arm_right_j4_m1.appendChild(doc.createTextNode(values[40]));
                        waypoint.appendChild(arm_right_j4_m1);
                        QDomElement arm_right_j4_m2 = doc.createElement("arm_right_j4_m2");
                        arm_right_j4_m2.appendChild(doc.createTextNode(values[41]));
                        waypoint.appendChild(arm_right_j4_m2);
                        QDomElement arm_right_j4_m3 = doc.createElement("arm_right_j4_m3");
                        arm_right_j4_m3.appendChild(doc.createTextNode(values[42]));
                        waypoint.appendChild(arm_right_j4_m3);
                        QDomElement arm_right_j4_m4 = doc.createElement("arm_right_j4_m4");
                        arm_right_j4_m4.appendChild(doc.createTextNode(values[43]));
                        waypoint.appendChild(arm_right_j4_m4);

                    } else if (item->text().contains("等待")) {
                        // 如果是延时，则保存延时信息
                        QDomElement delay = doc.createElement("Delay");
                        delay.appendChild(doc.createTextNode(item->data(Qt::UserRole).toString()));
                        waypoint.appendChild(delay);
                    }
                    root.appendChild(waypoint);
                }

                // 保存 XML 文件，确保使用 UTF-8 编码
                QFile file(fileName);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream stream(&file);
                    stream.setCodec("UTF-8");  // 设置编码为 UTF-8
                    stream << doc.toString();
                    file.close();
                }
            }
        });

        // 8.连接"读取路径"按钮
        connect(ui->pushButton_load, &QPushButton::clicked, this, [=]() {
            QString fileName = QFileDialog::getOpenFileName(this, "读取XML文件", "", "XML文件 (*.XML);;所有文件 (*.*)");
                QFile file(fileName);
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    // 使用 QFile 来读取 XML 内容并传递给 QDomDocument
                    QDomDocument doc;
                    if (!doc.setContent(&file)) {
                        file.close();
                        QMessageBox::warning(this, "错误", "无法解析 XML 文件");
                        return;
                    }
                    file.close();

                    ui->listWidget->clear();  // 清空当前的途径点列表

                    QDomElement root = doc.documentElement();
                    QDomNodeList waypoints = root.elementsByTagName("Waypoint");

                    // 遍历每一个 Waypoint 元素
                    for (int i = 0; i < waypoints.count(); ++i) {
                        QDomNode node = waypoints.at(i);
                        if (node.isElement()) {
                            QDomElement waypointElement = node.toElement();
                            QString name = waypointElement.attribute("name");
                            QListWidgetItem *item = new QListWidgetItem(name);

                            // 检查是否为延时节点
                            if (name.contains("等待")) {
                                QString delay = waypointElement.firstChildElement("Delay").text();
                                item->setData(Qt::UserRole, delay);  // 将延时信息存储到 UserRole
                                ui->listWidget->addItem(item);
                            } else {
                                // 读取电机和机械臂的参数并组合成字符串
                                QStringList dataList;

                                // 逐个检查每个节点是否存在，确保读取时不越界
                                auto addIfExists = [&](const QString &tagName) {
                                    QDomElement element = waypointElement.firstChildElement(tagName);
                                    if (!element.isNull()) {
                                        dataList.append(element.text());
                                    } else {
                                        dataList.append("0");  // 如果节点不存在，则添加默认值
                                    }
                                };

                                // 读取电机左侧参数
                                addIfExists("Motor_Left_1_ID");
                                addIfExists("Motor_Left_1_Position");
                                addIfExists("Motor_Left_1_Velocity");

                                addIfExists("Motor_Left_2_ID");
                                addIfExists("Motor_Left_2_Position");
                                addIfExists("Motor_Left_2_Velocity");

                                // 读取电机右侧参数
                                addIfExists("Motor_Right_1_ID");
                                addIfExists("Motor_Right_1_Position");
                                addIfExists("Motor_Right_1_Velocity");

                                addIfExists("Motor_Right_2_ID");
                                addIfExists("Motor_Right_2_Position");
                                addIfExists("Motor_Right_2_Velocity");

                                // 读取机械臂左侧参数
                                addIfExists(QString("arm_left_j1_m1"));
                                addIfExists(QString("arm_left_j1_m2"));
                                addIfExists(QString("arm_left_j1_m3"));
                                addIfExists(QString("arm_left_j1_m4"));

                                addIfExists(QString("arm_left_j2_m1"));
                                addIfExists(QString("arm_left_j2_m2"));
                                addIfExists(QString("arm_left_j2_m3"));
                                addIfExists(QString("arm_left_j2_m4"));

                                addIfExists(QString("arm_left_j3_m1"));
                                addIfExists(QString("arm_left_j3_m2"));
                                addIfExists(QString("arm_left_j3_m3"));
                                addIfExists(QString("arm_left_j3_m4"));

                                addIfExists(QString("arm_left_j4_m1"));
                                addIfExists(QString("arm_left_j4_m2"));
                                addIfExists(QString("arm_left_j4_m3"));
                                addIfExists(QString("arm_left_j4_m4"));

                                addIfExists(QString("arm_right_j1_m1"));
                                addIfExists(QString("arm_right_j1_m2"));
                                addIfExists(QString("arm_right_j1_m3"));
                                addIfExists(QString("arm_right_j1_m4"));

                                addIfExists(QString("arm_right_j2_m1"));
                                addIfExists(QString("arm_right_j2_m2"));
                                addIfExists(QString("arm_right_j2_m3"));
                                addIfExists(QString("arm_right_j2_m4"));

                                addIfExists(QString("arm_right_j3_m1"));
                                addIfExists(QString("arm_right_j3_m2"));
                                addIfExists(QString("arm_right_j3_m3"));
                                addIfExists(QString("arm_right_j3_m4"));

                                addIfExists(QString("arm_right_j4_m1"));
                                addIfExists(QString("arm_right_j4_m2"));
                                addIfExists(QString("arm_right_j4_m3"));
                                addIfExists(QString("arm_right_j4_m4"));

                                // 确保 dataList 的长度是 44
                                if (dataList.size() == 44) {
                                    // 将数据组合成字符串并存储到 UserRole
                                    QString dataString = dataList.join(",");
                                    item->setData(Qt::UserRole, dataString);
                                } else {
                                    QMessageBox::warning(this, "错误", "读取的参数数量不正确");
                                    return;
                                }

                                // 添加到列表
                                ui->listWidget->addItem(item);
                            }
                        }
                    }
                }
            });




        // 9.连接"重命名"按钮
        connect(ui->pushButton_rename, &QPushButton::clicked, this, [=]() {
            int currentRow = ui->listWidget->currentRow();  // 获取选中的 waypoint
            if (currentRow != -1) {
                QListWidgetItem *currentItem = ui->listWidget->item(currentRow);

                // 弹出输入框让用户输入新的名称
                bool ok;
                QString newName = QInputDialog::getText(this, "重命名",
                                                        "输入新的途径点名称：",
                                                        QLineEdit::Normal, currentItem->text(), &ok);
                if (ok && !newName.isEmpty()) {
                    // 设置新的名称
                    currentItem->setText(newName);
                }
            } else {
                // 如果没有选中任何waypoint，提示用户选择一个
                QMessageBox::warning(this, "提示", "请先选择一个途径点进行重命名");
            }
        });

        // 10.连接"写入"按钮的功能
        connect(ui->pushButton_write, &QPushButton::clicked, this, [=]() {
            int currentRow = ui->listWidget->currentRow();  // 获取当前选中的 waypoint
            if (currentRow != -1) {
                QListWidgetItem *currentItem = ui->listWidget->item(currentRow);

                // 获取所有 LineEdit 中的值，组合成字符串
                QString newData = QString(
                    "%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,"
                    "%21,%22,%23,%24,%25,%26,%27,%28,%29,%30,%31,%32,%33,%34,%35,%36,%37,%38,%39,%40,"
                    "%41,%42,%43,%44"
                ).arg(ui->LineEdit_motor_left_1_id->text())
                 .arg(ui->LineEdit_motor_left_1_position->text())
                 .arg(ui->LineEdit_motor_left_1_velocity->text())

                 .arg(ui->LineEdit_motor_left_2_id->text())
                 .arg(ui->LineEdit_motor_left_2_position->text())
                 .arg(ui->LineEdit_motor_left_2_velocity->text())

                 .arg(ui->LineEdit_motor_right_1_id->text())
                 .arg(ui->LineEdit_motor_right_1_position->text())
                 .arg(ui->LineEdit_motor_right_1_velocity->text())

                 .arg(ui->LineEdit_motor_right_2_id->text())
                 .arg(ui->LineEdit_motor_right_2_position->text())
                 .arg(ui->LineEdit_motor_right_2_velocity->text())


                 // 添加机械臂左侧的数据
                 .arg(ui->LineEdit_arm_left_j1_m1->text())
                 .arg(ui->LineEdit_arm_left_j1_m2->text())
                 .arg(ui->LineEdit_arm_left_j1_m3->text())
                 .arg(ui->LineEdit_arm_left_j1_m4->text())
                 .arg(ui->LineEdit_arm_left_j2_m1->text())
                 .arg(ui->LineEdit_arm_left_j2_m2->text())
                 .arg(ui->LineEdit_arm_left_j2_m3->text())
                 .arg(ui->LineEdit_arm_left_j2_m4->text())
                 .arg(ui->LineEdit_arm_left_j3_m1->text())
                 .arg(ui->LineEdit_arm_left_j3_m2->text())
                 .arg(ui->LineEdit_arm_left_j3_m3->text())
                 .arg(ui->LineEdit_arm_left_j3_m4->text())
                 .arg(ui->LineEdit_arm_left_j4_m1->text())
                 .arg(ui->LineEdit_arm_left_j4_m2->text())
                 .arg(ui->LineEdit_arm_left_j4_m3->text())
                 .arg(ui->LineEdit_arm_left_j4_m4->text())

                 // 添加机械臂右侧的数据
                 .arg(ui->LineEdit_arm_right_j1_m1->text())
                 .arg(ui->LineEdit_arm_right_j1_m2->text())
                 .arg(ui->LineEdit_arm_right_j1_m3->text())
                 .arg(ui->LineEdit_arm_right_j1_m4->text())
                 .arg(ui->LineEdit_arm_right_j2_m1->text())
                 .arg(ui->LineEdit_arm_right_j2_m2->text())
                 .arg(ui->LineEdit_arm_right_j2_m3->text())
                 .arg(ui->LineEdit_arm_right_j2_m4->text())
                 .arg(ui->LineEdit_arm_right_j3_m1->text())
                 .arg(ui->LineEdit_arm_right_j3_m2->text())
                 .arg(ui->LineEdit_arm_right_j3_m3->text())
                 .arg(ui->LineEdit_arm_right_j3_m4->text())
                 .arg(ui->LineEdit_arm_right_j4_m1->text())
                 .arg(ui->LineEdit_arm_right_j4_m2->text())
                 .arg(ui->LineEdit_arm_right_j4_m3->text())
                 .arg(ui->LineEdit_arm_right_j4_m4->text());

                // 将新值更新到 UserRole
                currentItem->setData(Qt::UserRole, newData);

            } else {
                // 如果没有选中任何 waypoint，提示用户选择一个
                QMessageBox::warning(this, "提示", "请先选择一个途径点进行修改");
            }
        });

        // 11.使能所有电机
        connect(ui->pushButton_enable_all_motor, &QPushButton::clicked, this, [=]() {
            // for (int8_t MOTOR_id = 1; MOTOR_id <= 4; MOTOR_id++) {//电机序号从1开始而非0，参考云深处多电机控制example
            //     int8_t cmd = ENABLE_MOTOR;
            //     int16_t id;
            //     id = (cmd << CAN_ID_SHIFT_BITS) | MOTOR_id;
            for (int8_t MOTOR_id = 1; MOTOR_id <= 4; MOTOR_id++) {//电机序号从1开始而非0，参考云深处多电机控制example
                // int8_t cmd = ENABLE_MOTOR;
                // int16_t id;
                // id = (cmd << CAN_ID_SHIFT_BITS) | MOTOR_id;
                //1.1创建一个motorTarget消息
                ihb_msgs::msg::ServoMotorJointTarget motorTarget;
                motorTarget.id = MOTOR_id;
                motorTarget.cmd = ENABLE_ALL_MOTOR;
                //1.2 获取全局信号发送器
                SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
                //1.3 发送消息
                emit signal_transmitter->ihbServoMotorTargetChangedSignal(motorTarget);
                //2.1调试输出
                //2.2获取当前系统时间
                QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                QStringList debugOutput;
                debugOutput << "MOTOR " + QString::number(MOTOR_id, 16).toUpper() + " ENABLE ";
                //2.3使用 qDebug 逐行输出到控制台
                for (const QString &line : debugOutput) {
                    qDebug() << line;
                }
                //2.4添加时间戳到 QTextEdit 输出，并加入换行和分隔线
                QString outputWithTime = "[" + currentTime + "]\n" + debugOutput.join("\n") + "\n------------------------------------";
                //2.5使用 appendToTextEdit 输出到 QTextEdit
                appendToTextEdit(outputWithTime);
            }
            qDebug("%s","ENABLE ALL MOTOR");
        });

        // 12.失能所有电机
        connect(ui->pushButton_disable_all_motor, &QPushButton::clicked, this, [=]() {
            for (int8_t MOTOR_id = 1; MOTOR_id <= 4; MOTOR_id++) {
                // int8_t cmd = DISABLE_MOTOR;
                // int16_t id;
                // id = (cmd << CAN_ID_SHIFT_BITS) | MOTOR_id;
                //1.1创建一个motorTarget消息
                ihb_msgs::msg::ServoMotorJointTarget motorTarget;
                motorTarget.id = MOTOR_id;
                motorTarget.cmd = DISABLE_ALL_MOTOR;
                //1.2 获取全局信号发送器
                SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
                //1.3 发送消息
                emit signal_transmitter->ihbServoMotorTargetChangedSignal(motorTarget);
                //2.1调试输出
                //2.2获取当前系统时间
                QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                QStringList debugOutput;
                debugOutput << "MOTOR " + QString::number(MOTOR_id, 16).toUpper() + " DISABLE ";
                //2.3使用 qDebug 逐行输出到控制台
                for (const QString &line : debugOutput) {
                    qDebug() << line;
                }
                //2.4添加时间戳到 QTextEdit 输出，并加入换行和分隔线
                QString outputWithTime = "[" + currentTime + "]\n" + debugOutput.join("\n") + "\n------------------------------------";
                //2.5使用 appendToTextEdit 输出到 QTextEdit
                appendToTextEdit(outputWithTime);
            }
            qDebug("%s","DISABLE ALL MOTORS");
        });
        // 13.发送单点位置，一键发送
        connect(ui->pushButton_runSingleWaypoint, &QPushButton::clicked, this, [=]() {
            on_pushButton_motor_left_1_send_clicked();
            on_pushButton_motor_left_2_send_clicked();
            on_pushButton_motor_right_1_send_clicked();
            on_pushButton_motor_right_2_send_clicked();
            on_pushButton_arm_left_send_clicked();
            on_pushButton_arm_right_send_clicked();
        });
        // 14.// 连接"停止并归零"按钮
        connect(ui->pushButton_stopBackToZero, &QPushButton::clicked, this, &teach::processStopBackToZero);

        //15.0运行按钮辅助配置
        // 配置 waypointTimer
        // waypointTimer->setSingleShot(true);
        // connect(waypointTimer, &QTimer::timeout, this, &teach::onWaypointTimeout);
        // 连接自定义信号 waypointReached 到槽函数 onWaypointReached
        // connect(this, &teach::waypointReached, this, &teach::onWaypointReached);
        //15.1运行按钮
        connect(ui->pushButton_run, &QPushButton::clicked, this, [=]() {
            //--------说明-------------------
            // 按钮点击事件：在按钮的点击事件中，检查当前状态并进行相应操作（发送waypoint或停止发送）。
            // 发送waypoints：使用一个循环或队列依次发送waypoint，并在每次发送后添加一个非阻塞的延时。
            // 改变按钮文本：在按钮按下时，切换按钮文本为“Stop”，并在停止时返回为“Run”。
            // 循环发送：根据 checkBox_loop 的状态来决定是否要重复发送。
            //-------------------------------
            //0清空之前的waypoint数据
            waypointsData.clear();
            // 1检查当前是否在运行并切换按钮文本
            if (!isRunning) {
                isRunning = true;
                ui->pushButton_run->setText(" ⏯️ Stop");
                ui->pushButton_run->setStyleSheet("background-color: red;");
                //15.1.2将所有 SEND 按钮禁用
                ui->pushButton_motor_left_1_send->setEnabled(false);
                ui->pushButton_motor_left_2_send->setEnabled(false);
                ui->pushButton_motor_right_1_send->setEnabled(false);
                ui->pushButton_motor_right_2_send->setEnabled(false);
                ui->pushButton_arm_left_send->setEnabled(false);
                ui->pushButton_arm_right_send->setEnabled(false);
                ui->pushButton_def->setEnabled(false);
                ui->pushButton_add_forw->setEnabled(false);
                ui->pushButton_add_back->setEnabled(false);
                ui->pushButton_del->setEnabled(false);
                ui->pushButton_add_delay->setEnabled(false);
                ui->pushButton_save->setEnabled(false);
                ui->pushButton_load->setEnabled(false);
                ui->pushButton_rename->setEnabled(false);
                ui->pushButton_write->setEnabled(false);
                ui->pushButton_enable_all_motor->setEnabled(false);
                ui->pushButton_disable_all_motor->setEnabled(false);
                ui->pushButton_runSingleWaypoint->setEnabled(false);
                ui->pushButton_stopBackToZero->setEnabled(false);
                currentWaypointIndex = 0;
                // 15.1.3
                //15.1.4创建worker线程
                workerThread = new QThread();
                worker_ptr = new worker(this);
                worker_ptr->moveToThread(workerThread);
                //15.1.5连接信号和槽
                connect(workerThread, &QThread::started, worker_ptr, &worker::process);
                connect(worker_ptr, &worker::finished, this, &teach::handleWorkerFinished);
                connect(worker_ptr, &worker::finished, workerThread, &QThread::quit);
                connect(worker_ptr, &worker::finished, worker_ptr, &worker::deleteLater);
                connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
                connect(worker_ptr, &worker::logMessage, this, &teach::handleLogMessage);
                //15.1.6启动线程
                workerThread->start();

                //15.1.7提取所有waypoint到全局变量中供worker使用
                for (int i = 0; i < ui->listWidget->count(); ++i) {
                    // QListWidgetItem *item = ui->listWidget->item(i);
                    // QVariantMap waypointData = item->data(Qt::UserRole).toMap();
                    //waypointsData.append(waypointData);  // 将 waypoint 数据添加到全局变量
                    QStringList waypoint = ui->listWidget->item(i)->data(Qt::UserRole).toString().split(",");
                    waypointsData.append(waypoint);
                }
                loop = ui->checkBox_loop->isChecked();}

                //15.1.8发送信号给 worker 线程处理 waypointsData
            else {
                isRunning = false;
                handleWorkerFinished();
                if (worker_ptr) {worker_ptr->stop();}
                processStopBackToZero(); // 停止,返回零点
            }
        });
        //16.定时查询电机是否在线
        // connect(motorStatusTimer, &QTimer::timeout, this, &teach::updateMotorIsOnline);
        // motorStatusTimer->start(500); // 每隔500ms查询一次
        //16.在worker线程结束时，通过信号触发handleWorkerFinished()
        connect(worker_ptr, &worker::finished, this, &teach::handleWorkerFinished);

}

// worker完成信号
void teach::handleWorkerFinished()
{
    isRunning = false;
    ui->pushButton_run->setText(" ⏯️  Run");
    ui->pushButton_run->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(0, 200, 0, 255), stop:1 rgba(255, 255, 255, 255));");
    // 启用 SEND 按钮
    ui->pushButton_motor_left_1_send->setEnabled(true);
    ui->pushButton_motor_left_2_send->setEnabled(true);
    ui->pushButton_motor_right_1_send->setEnabled(true);
    ui->pushButton_motor_right_2_send->setEnabled(true);
    ui->pushButton_arm_left_send->setEnabled(true);
    ui->pushButton_arm_right_send->setEnabled(true);
    ui->pushButton_def->setEnabled(true);
    ui->pushButton_add_forw->setEnabled(true);
    ui->pushButton_add_back->setEnabled(true);
    ui->pushButton_del->setEnabled(true);
    ui->pushButton_add_delay->setEnabled(true);
    ui->pushButton_save->setEnabled(true);
    ui->pushButton_load->setEnabled(true);
    ui->pushButton_rename->setEnabled(true);
    ui->pushButton_write->setEnabled(true);
    ui->pushButton_enable_all_motor->setEnabled(true);
    ui->pushButton_disable_all_motor->setEnabled(true);
    ui->pushButton_runSingleWaypoint->setEnabled(true);
    ui->pushButton_stopBackToZero->setEnabled(true);
    appendToTextEdit("RUNNING FINISH");
}

// 处理日志消息
void teach::handleLogMessage(const QString &message)
{
    appendToTextEdit(message);
}
//运行按钮（15）辅助函数 1
void teach::run_SendWaypoints() {
    while (isRunning){
        // 0.检查是否到达列表末尾
        if (currentWaypointIndex >= waypointsData.size()) {
            if (loop) {
                currentWaypointIndex = 0; // 如果需要循环，重置索引
            } else {
                // 不需要循环，停止运行
                isRunning = false;
                //handleWorkerFinished();
                return;
            }
        }
        // 1.从 listWidget 获取 第一个waypoint的信息  waypointData
        // QListWidgetItem* item = ui->listWidget->item(currentWaypointIndex);
        // QString itemText = item->text();

        // QString itemText = waypointsData[currentWaypointIndex] -> text();
        // QString itemText = waypointsData[currentWaypointIndex]["text"].toString();
        // QString itemText = waypointsData[currentWaypointIndex];
        QString itemText = waypointsData[currentWaypointIndex].join(", ");

        // 1.1判断是延时还是waypoint
        if (itemText.contains("等待")) {
            // 1.1.1这是一个延时项，解析延时时间
            QRegExp regExp("等待\\s*(\\d+(\\.\\d+)?)\\s*秒");
            if (regExp.indexIn(itemText) != -1) {
                double delaySeconds = regExp.cap(1).toDouble();
                // 使用 QTimer 进行非阻塞延时
                //QTimer::singleShot(static_cast<int>(delaySeconds * 1000), this, SLOT(sendNextWaypoint()));
                if (!waitForDelayWithCondition(delaySeconds)) {//延时
                    break;// 但是如果收到STOP请求，结束延时，提前退出
                }
            } else {
                // 1.1.2格式错误，直接发送下一个
                logMessage(QString("ERR:10 延时格式错误\n------------------------------------"));
                currentWaypointIndex++;
            }
        } else {
            // 1.2.1这是一个正常的 waypoint，获取数据并发送
            // QString dataString = item->data(Qt::UserRole).toString();
            // QStringList values = dataString.split(",");

            //参考
            // //itemclick
            // QStringList values = item->data(Qt::UserRole).toString().split(",");
            // //get waypoint
            // QStringList waypoint = ui->listWidget->item(i)->data(Qt::UserRole).toString().split(",");
            //                     waypointsData.append(waypoint);

            QStringList values = waypointsData[currentWaypointIndex];
            if (values.size() == 44) {
                // 1.2.2调用发送函数发送数据
                sendWaypointData(values);
            } else {
                // 1.2.3数据不完整，跳过
                logMessage(QString("WARNING:11 数据不完整，跳过该点\n------------------------------------"));
                currentWaypointIndex++;
                //return;
            }
            // // 设置标志位为 false，等待下位机发送确认信号
            // isWaypointReached = false;
            // // 启动定时器，等待机器人确认或超时
            // int timeoutMs = 5000; // 获取超时时间（毫秒）
            // waypointTimer->start(timeoutMs);
        }
        currentWaypointIndex++; // 准备发送下一个 waypoint
    }
    emit worker_ptr->finished();// 发送完成信号
}

//运行按钮（15）辅助函数 2
void teach::sendWaypointData(const QStringList &values) {
    // 为每个电机和机械臂创建临时的 QLineEdit，并设置值
    // 电机左侧第一个
    QLineEdit idLineEdit1, positionLineEdit1, velocityLineEdit1;
    idLineEdit1.setText(values[0]);
    positionLineEdit1.setText(values[1]);
    velocityLineEdit1.setText(values[2]);
    processInput(&idLineEdit1, &positionLineEdit1, &velocityLineEdit1);
    // 电机左侧第二个
    QLineEdit idLineEdit2, positionLineEdit2, velocityLineEdit2;
    idLineEdit2.setText(values[3]);
    positionLineEdit2.setText(values[4]);
    velocityLineEdit2.setText(values[5]);
    processInput(&idLineEdit2, &positionLineEdit2, &velocityLineEdit2);

    // 电机右侧第一个
    QLineEdit idLineEdit3, positionLineEdit3, velocityLineEdit3;
    idLineEdit3.setText(values[6]);
    positionLineEdit3.setText(values[7]);
    velocityLineEdit3.setText(values[8]);
    processInput(&idLineEdit3, &positionLineEdit3, &velocityLineEdit3);

    // 电机右侧第二个
    QLineEdit idLineEdit4, positionLineEdit4, velocityLineEdit4;
    idLineEdit4.setText(values[9]);
    positionLineEdit4.setText(values[10]);
    velocityLineEdit4.setText(values[11]);

    processInput(&idLineEdit4, &positionLineEdit4, &velocityLineEdit4);
    // 机械臂左侧
    QList<QLineEdit*> leftArmLineEdits;
    for (int i = 12; i < 28; ++i) {
        QLineEdit *lineEdit = new QLineEdit();
        lineEdit->setText(values[i]);
        leftArmLineEdits.append(lineEdit);
    }
    processInput_valve(true,
                       leftArmLineEdits[0], leftArmLineEdits[1], leftArmLineEdits[2], leftArmLineEdits[3],
                       leftArmLineEdits[4], leftArmLineEdits[5], leftArmLineEdits[6], leftArmLineEdits[7],
                       leftArmLineEdits[8], leftArmLineEdits[9], leftArmLineEdits[10], leftArmLineEdits[11],
                       leftArmLineEdits[12], leftArmLineEdits[13], leftArmLineEdits[14], leftArmLineEdits[15],
                       new QLineEdit("350")); // 假设压力限制为 350

    // 机械臂右侧
    QList<QLineEdit*> rightArmLineEdits;
    for (int i = 28; i < 44; ++i) {
        QLineEdit *lineEdit = new QLineEdit();
        lineEdit->setText(values[i]);
        rightArmLineEdits.append(lineEdit);
    }
    processInput_valve(false,
                       rightArmLineEdits[0], rightArmLineEdits[1], rightArmLineEdits[2], rightArmLineEdits[3],
                       rightArmLineEdits[4], rightArmLineEdits[5], rightArmLineEdits[6], rightArmLineEdits[7],
                       rightArmLineEdits[8], rightArmLineEdits[9], rightArmLineEdits[10], rightArmLineEdits[11],
                       rightArmLineEdits[12], rightArmLineEdits[13], rightArmLineEdits[14], rightArmLineEdits[15],
                       new QLineEdit("350")); // 假设压力限制为 0

    // 释放动态创建的 QLineEdit
    qDeleteAll(leftArmLineEdits);
    qDeleteAll(rightArmLineEdits);
}

void teach::processStopBackToZero(){
    // 定义零值字符串
    QLineEdit *lineEdit_zero = new QLineEdit();
    QLineEdit *lineEdit_SpeedLimit = new QLineEdit();
    lineEdit_zero->setText("0");
    lineEdit_SpeedLimit->setText("150");
    // 直接调用发送函数，将所有数据设为零而不影响 UI 上的值
    processInput(ui->LineEdit_motor_left_1_id,  lineEdit_zero, lineEdit_SpeedLimit);
    processInput(ui->LineEdit_motor_left_2_id,  lineEdit_zero, lineEdit_SpeedLimit);
    processInput(ui->LineEdit_motor_right_1_id, lineEdit_zero, lineEdit_SpeedLimit);
    processInput(ui->LineEdit_motor_right_2_id, lineEdit_zero, lineEdit_SpeedLimit);
    // 设置机械臂的气压值为零
    QLineEdit *lineEdit_pressureLimit = new QLineEdit();
    lineEdit_pressureLimit->setText("350");// 假设压力限制为350
    processInput_valve(true, lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                             lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                             lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                             lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                             lineEdit_pressureLimit);
    processInput_valve(false, lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                              lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                              lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                              lineEdit_zero, lineEdit_zero, lineEdit_zero, lineEdit_zero,
                              lineEdit_pressureLimit);
    delete lineEdit_zero, lineEdit_pressureLimit;
}

// 当运行延时步骤时，如果外部按下STOP，能及时停止延时
bool teach::waitForDelayWithCondition(double delaySeconds)
{
    // QMutex mutex;
    // QWaitCondition waitCondition;

    // mutex.lock();
    // bool stopped = !waitCondition.wait(&mutex, static_cast<int>(delaySeconds * 1000));
    // mutex.unlock();

    // return isRunning && !stopped;
    QTime endTime = QTime::currentTime().addMSecs(static_cast<int>(delaySeconds * 1000));
    while (QTime::currentTime() < endTime) {
        if (!isRunning)
            return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        QThread::msleep(100);
    }
    return true;
}

// 辅助函数，用于将文本追加到 QTextEdit
void teach::appendToTextEdit(const QString &text)
{
    ui->OutputWindow->append(text); // 将文本追加到 QTextEdit
}

void teach::on_pushButton_motor_left_1_send_clicked()
{
    processInput(ui->LineEdit_motor_left_1_id,
                 ui->LineEdit_motor_left_1_position,
                 ui->LineEdit_motor_left_1_velocity);
}

void teach::on_pushButton_motor_left_2_send_clicked()
{
    processInput(ui->LineEdit_motor_left_2_id,
                 ui->LineEdit_motor_left_2_position,
                 ui->LineEdit_motor_left_2_velocity);
}

void teach::on_pushButton_motor_right_1_send_clicked()
{
    processInput(ui->LineEdit_motor_right_1_id,
                 ui->LineEdit_motor_right_1_position,
                 ui->LineEdit_motor_right_1_velocity);
}

void teach::on_pushButton_motor_right_2_send_clicked()
{
    processInput(ui->LineEdit_motor_right_2_id,
                 ui->LineEdit_motor_right_2_position,
                 ui->LineEdit_motor_right_2_velocity);
}

void teach::on_pushButton_arm_left_send_clicked()
{
    processInput_valve( true,
                        ui->LineEdit_arm_left_j1_m1,
                        ui->LineEdit_arm_left_j1_m2,
                        ui->LineEdit_arm_left_j1_m3,
                        ui->LineEdit_arm_left_j1_m4,
                        ui->LineEdit_arm_left_j2_m1,
                        ui->LineEdit_arm_left_j2_m2,
                        ui->LineEdit_arm_left_j2_m3,
                        ui->LineEdit_arm_left_j2_m4,
                        ui->LineEdit_arm_left_j3_m1,
                        ui->LineEdit_arm_left_j3_m2,
                        ui->LineEdit_arm_left_j3_m3,
                        ui->LineEdit_arm_left_j3_m4,
                        ui->LineEdit_arm_left_j4_m1,
                        ui->LineEdit_arm_left_j4_m2,
                        ui->LineEdit_arm_left_j4_m3,
                        ui->LineEdit_arm_left_j4_m4,
                        ui->LineEdit_arm_pressure_limit
                        );
}

void teach::on_pushButton_arm_right_send_clicked()
{
    processInput_valve( false,
                        ui->LineEdit_arm_right_j1_m1,
                        ui->LineEdit_arm_right_j1_m2,
                        ui->LineEdit_arm_right_j1_m3,
                        ui->LineEdit_arm_right_j1_m4,
                        ui->LineEdit_arm_right_j2_m1,
                        ui->LineEdit_arm_right_j2_m2,
                        ui->LineEdit_arm_right_j2_m3,
                        ui->LineEdit_arm_right_j2_m4,
                        ui->LineEdit_arm_right_j3_m1,
                        ui->LineEdit_arm_right_j3_m2,
                        ui->LineEdit_arm_right_j3_m3,
                        ui->LineEdit_arm_right_j3_m4,
                        ui->LineEdit_arm_right_j4_m1,
                        ui->LineEdit_arm_right_j4_m2,
                        ui->LineEdit_arm_right_j4_m3,
                        ui->LineEdit_arm_right_j4_m4,
                        ui->LineEdit_arm_pressure_limit
                        );
}
void teach::on_pushButton_motor_left_1_toZero_clicked()
{
    ui->LineEdit_motor_left_1_position->setText("0");
}

void teach::on_pushButton_motor_left_2_toZero_clicked()
{
    ui->LineEdit_motor_left_2_position->setText("0");
}

void teach::on_pushButton_motor_right_1_toZero_clicked()
{
    ui->LineEdit_motor_right_1_position->setText("0");
}

void teach::on_pushButton_motor_right_2_toZero_clicked()
{
    ui->LineEdit_motor_right_2_position->setText("0");
}

void teach::on_pushButton_arm_left_toZero_clicked()
{
    ui->LineEdit_arm_left_j1_m1->setText("0");
    ui->LineEdit_arm_left_j1_m2->setText("0");
    ui->LineEdit_arm_left_j1_m3->setText("0");
    ui->LineEdit_arm_left_j1_m4->setText("0");
    ui->LineEdit_arm_left_j2_m1->setText("0");
    ui->LineEdit_arm_left_j2_m2->setText("0");
    ui->LineEdit_arm_left_j2_m3->setText("0");
    ui->LineEdit_arm_left_j2_m4->setText("0");
    ui->LineEdit_arm_left_j3_m1->setText("0");
    ui->LineEdit_arm_left_j3_m2->setText("0");
    ui->LineEdit_arm_left_j3_m3->setText("0");
    ui->LineEdit_arm_left_j3_m4->setText("0");
    ui->LineEdit_arm_left_j4_m1->setText("0");
    ui->LineEdit_arm_left_j4_m2->setText("0");
    ui->LineEdit_arm_left_j4_m3->setText("0");
    ui->LineEdit_arm_left_j4_m4->setText("0");
    ui->LineEdit_arm_left_CH16->setText("0");
    ui->LineEdit_arm_left_CH17->setText("0");
    ui->LineEdit_arm_left_CH18->setText("0");
    ui->LineEdit_arm_left_CH19->setText("0");
    ui->LineEdit_arm_left_CH20->setText("0");
    ui->LineEdit_arm_left_CH21->setText("0");
    ui->LineEdit_arm_left_CH22->setText("0");
    ui->LineEdit_arm_left_CH23->setText("0");
}

void teach::on_pushButton_arm_right_toZero_clicked()
{
    ui->LineEdit_arm_right_j1_m1->setText("0");
    ui->LineEdit_arm_right_j1_m2->setText("0");
    ui->LineEdit_arm_right_j1_m3->setText("0");
    ui->LineEdit_arm_right_j1_m4->setText("0");
    ui->LineEdit_arm_right_j2_m1->setText("0");
    ui->LineEdit_arm_right_j2_m2->setText("0");
    ui->LineEdit_arm_right_j2_m3->setText("0");
    ui->LineEdit_arm_right_j2_m4->setText("0");
    ui->LineEdit_arm_right_j3_m1->setText("0");
    ui->LineEdit_arm_right_j3_m2->setText("0");
    ui->LineEdit_arm_right_j3_m3->setText("0");
    ui->LineEdit_arm_right_j3_m4->setText("0");
    ui->LineEdit_arm_right_j4_m1->setText("0");
    ui->LineEdit_arm_right_j4_m2->setText("0");
    ui->LineEdit_arm_right_j4_m3->setText("0");
    ui->LineEdit_arm_right_j4_m4->setText("0");
    ui->LineEdit_arm_right_CH16->setText("0");
    ui->LineEdit_arm_right_CH17->setText("0");
    ui->LineEdit_arm_right_CH18->setText("0");
    ui->LineEdit_arm_right_CH19->setText("0");
    ui->LineEdit_arm_right_CH20->setText("0");
    ui->LineEdit_arm_right_CH21->setText("0");
    ui->LineEdit_arm_right_CH22->setText("0");
    ui->LineEdit_arm_right_CH23->setText("0");
}

// void teach::on_pushButton_SetCan0Up_clicked()
// {
//     if (isCanInterfaceUp()) {
//         QMessageBox::information(this, "Info", "CAN0 interface is already up.");
//         return;
//     }
//     process->start("sudo", {"ip", "link", "set", "can0", "up", "type", "can", "bitrate", "2000000"});
//     QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
//     QString outputWithTime = "[" + currentTime + "]\n" + "SET CAN0 UP \n------------------------------------";
//     appendToTextEdit(outputWithTime);
// }

// void teach::on_pushButton_SetCan0Down_clicked()
// {
//     process->start("sudo", {"ip", "link", "set", "can0", "down"});
//     QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
//     QString outputWithTime = "[" + currentTime + "]\n" + "SET CAN0 DOWN \n------------------------------------";
//     appendToTextEdit(outputWithTime);
// }

// bool teach::isCanInterfaceUp()
// {
// //    process->start("ip", {"link", "show", "can0"});
// //    process->waitForFinished();
// //    QString output = process->readAllStandardOutput();
// //    return output.contains("UP");
//     // 改为使用 execute 而不是 start
//         int exitCode = QProcess::execute("ip", {"link", "show", "can0"});
//         if (exitCode != 0) {
//             return false; // 如果命令执行失败，返回 false
//         }

//         // 检查命令输出
//         QString output = process->readAllStandardOutput();
//         return output.contains("UP");
// }

//电机CAN发送按钮辅助函数
void teach::processInput(QLineEdit *idLineEdit, QLineEdit *positionLineEdit, QLineEdit *velocityLineEdit)
{
    // 1. 获取数值并验证
    bool ok;
    int canID = idLineEdit->text().toInt(&ok, 16);
    if (!ok || canID < 0 || canID > 0x1FF) {
        QMessageBox::warning(this, "Input Error", "Invalid CAN ID, must be an integer between 0 and 0x1FF");
        return;
    }
    float position = positionLineEdit->text().toFloat(&ok);
    if (!ok || position < -180.0f || position > 360.0f) {
        QMessageBox::warning(this, "Input Error", "Position must be a number between -40 and 40");
        return;
    }
    float velocity = velocityLineEdit->text().toFloat(&ok);
    if (!ok || velocity < 0.0f || velocity > 300.0f) {
        QMessageBox::warning(this, "Input Error", "Velocity limit must be a number between 0 and 300");
        return;
    }
    // 2. 转换数据
    // uint16_t targetAngle = static_cast<uint16_t>(((position + 40.0f) / 80.0f) * 65535.0f);
    // uint16_t targetVelocity = static_cast<uint16_t>((velocity + 40.0f) / 80.0f * 16383.0f);
    // uint16_t mappedKp = static_cast<uint16_t>(kp);
    // uint8_t  mappedKd = static_cast<uint8_t>(kd / 51.0f * 255.0f);
    // uint16_t targetTorque = static_cast<uint16_t>(((torque + 40.0f) / 80.0f) * 65535.0f);

    uint32_t targetAngle = static_cast<uint32_t>(position * 100);
    uint16_t targetVelocity = static_cast<uint16_t>(velocity);
    // 3. 创建并发送CAN消息
    sendCanMessage(canID, targetAngle, targetVelocity);
}

void teach::sendCanMessage(int canID, uint32_t targetAngle, uint16_t targetVelocity)
{
    //1.创建一个motorTarget消息
        ihb_msgs::msg::ServoMotorJointTarget motorTarget;
        motorTarget.id = canID;
        motorTarget.cmd = SET_TARGET;
        motorTarget.angle =       targetAngle;
        motorTarget.speed_limit = targetVelocity;
        //1.2 获取全局信号发送器
        SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
        //1.3 发送消息
        emit signal_transmitter->ihbServoMotorTargetChangedSignal(motorTarget);
    //2.输出调试信息
    logCanMessage_motor(canID, targetAngle, targetVelocity);

}

void teach::processInput_valve(bool isLeftArm,
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
                                QLineEdit *LineEdit_pressure_limit)
{
    // 1. 获取 LineEdit_pressure_limit 的值
    bool ok;
    int pressureLimit = LineEdit_pressure_limit->text().toInt(&ok);
    if (!ok || pressureLimit < 0) {
        QMessageBox::warning(this, "Input Error", "Pressure limit must be a non-negative integer");
        return;
    }

    // 2. 获取每个 LineEdit 的值并验证范围，然后转换为 int16_t
    QLineEdit* lineEdits[] = {
        LineEdit_arm_left_j1_m1, LineEdit_arm_left_j1_m2, LineEdit_arm_left_j1_m3, LineEdit_arm_left_j1_m4,
        LineEdit_arm_left_j2_m1, LineEdit_arm_left_j2_m2, LineEdit_arm_left_j2_m3, LineEdit_arm_left_j2_m4,
        LineEdit_arm_left_j3_m1, LineEdit_arm_left_j3_m2, LineEdit_arm_left_j3_m3, LineEdit_arm_left_j3_m4,
        LineEdit_arm_left_j4_m1, LineEdit_arm_left_j4_m2, LineEdit_arm_left_j4_m3, LineEdit_arm_left_j4_m4
    };
    uint16_t pressures[4][4];
    for (int jointID = 0; jointID < 4; ++jointID) {
        for (int muscleID = 0; muscleID < 4; ++muscleID) {
            int index = jointID * 4 + muscleID;
            int value = lineEdits[index]->text().toInt(&ok);
            if (!ok || value < 0 || value > pressureLimit) {
                QMessageBox::warning(this, "Input Error", QString("Pressure value for jointID %1 MuscleID %2 must be between 0 and %3").arg(jointID + 1).arg(muscleID + 1).arg(pressureLimit));
                return;
            }
            pressures[jointID][muscleID] = static_cast<int16_t>(value);
        }
        // 3.发送气压数据
        sendCanMessage_valve(isLeftArm, jointID, pressures[jointID]);
    }
    // 4.输出调试信息
    logCanMessage_valve(isLeftArm, pressures);
}

void teach::sendCanMessage_valve(bool isLeftArm, int jointid, uint16_t pressure[4])
{
    int canID;
    if(isLeftArm == true){
        canID = jointid;
    } 
    else{
        canID =4+jointid;
    }
    //1创建一个muscleTarget消息
    ihb_msgs::msg::MuscleJointTarget muscleTarget;
    muscleTarget.id = canID;
    muscleTarget.cmd = OPEN_LOOP;
    for (int i = 0; i < 4; i++)
    {
        muscleTarget.pressure[i] = pressure[i];
    }
    
    //2获取全局信号发送器
    SignalTransmitter *signal_transmitter = SignalTransmitter::instance();
    //3发送消息
    emit signal_transmitter->ihbMuscleTargetChangedSignal(muscleTarget);
}

void teach::logCanMessage_motor(int canID, uint32_t targetAngle, uint16_t targetVelocity)
{
    // 获取当前系统时间
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QStringList debugOutput;
    debugOutput << "CAN ID = " + QString::number(canID, 16).toUpper();
    debugOutput << "Target Angle = " + QString::number(targetAngle);
    debugOutput << "Speed Limit = " + QString::number(targetVelocity);
    // debugOutput << "Target Velocity = " + QString::number(targetVelocity);
    // debugOutput << "Kp = " + QString::number(mappedKp);
    // debugOutput << "Kd = " + QString::number(mappedKd);
    // debugOutput << "Target Torque = " + QString::number(targetTorque);
    // 使用 qDebug 逐行输出到控制台
    qDebug() << currentTime;
    for (const QString &line : debugOutput) {
        qDebug() << line;
    }
    // // 添加时间戳到 QTextEdit 输出，并加入换行和分隔线
    // QString outputWithTime = "[" + currentTime + "]\n" + debugOutput.join("\n") + "\n------------------------------------";
    // appendToTextEdit(outputWithTime);
}

void teach::logCanMessage_valve(bool isLeftArm, uint16_t pressures[4][4])
{
    // 获取当前系统时间
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QStringList debugOutput;
    //判断是否是左臂
    if (isLeftArm == true){debugOutput << "Left Arm ";}
        else{debugOutput << "Right Arm ";}
    for (int jointID = 0; jointID < 4; ++jointID) {
            debugOutput <<QString("Joint %1 ").arg(jointID)      + "  " +
                          QString::number(pressures[jointID][0]) + "  " +
                          QString::number(pressures[jointID][1]) + "  " +
                          QString::number(pressures[jointID][2]) + "  " +
                          QString::number(pressures[jointID][3]);}
    // 使用 qDebug 逐行输出到控制台
    for (const QString &line : debugOutput) {
        qDebug() << line;
    }
    // 添加时间戳到 QTextEdit 输出，并加入换行和分隔线
    // QString outputWithTime = "[" + currentTime + "]\n" + debugOutput.join("\n") + "\n------------------------------------";
    // appendToTextEdit(outputWithTime);
}

void teach::logMessage(QString log)
{
    // 常规日志
    // 获取当前系统时间
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QStringList debugOutput;
    debugOutput << log;
    // 使用 qDebug 逐行输出到控制台
    for (const QString &line : debugOutput) {
        qDebug() << line;
    }
    // 添加时间戳到 QTextEdit 输出，并加入换行和分隔线
    QString outputWithTime = "[" + currentTime + "]\n" + debugOutput.join("\n") + "\n------------------------------------";
    appendToTextEdit(outputWithTime);
}

void teach::updateMotorIsOnline(bool motor1status, bool motor2status, bool motor3status, bool motor4status)
{
    // qDebug("%s","-----------CHECKING MOTOR-----------------");
    if (motor1status) {
            // qDebug("%s","----------- motor 1 green -----------------");
            ui -> label_motor_left_1_status->setStyleSheet("background-color: green; border-radius: 10px;");
        }
        else {
            // qDebug("%s","----------- motor 1 red -----------------");
            ui -> label_motor_left_1_status->setStyleSheet("background-color: red; border-radius: 10px;");
        }
    if (motor2status) {
            ui -> label_motor_left_2_status->setStyleSheet("background-color: green; border-radius: 10px;");
    }
        else {
            ui -> label_motor_left_2_status->setStyleSheet("background-color: red; border-radius: 10px;");
        }
    if (motor3status) {
            ui -> label_motor_right_1_status->setStyleSheet("background-color: green; border-radius: 10px;");
    }
        else {
            ui -> label_motor_right_1_status->setStyleSheet("background-color: red; border-radius: 10px;");
        }
    if (motor4status) {
            ui -> label_motor_right_2_status->setStyleSheet("background-color: green; border-radius: 10px;");
    }
        else {
            ui -> label_motor_right_2_status->setStyleSheet("background-color: red; border-radius: 10px;");
        }
}

teach::~teach()
{
    if (workerThread && workerThread->isRunning()) {
        worker_ptr->stop();
        workerThread->quit();
        workerThread->wait();
    }
    process->terminate(); // 确保 QProcess 被正确终止
    delete ui;
    if (process) {
        process->terminate();
        delete process;
    }

}



