#!/bin/bash

# ROS2 机械臂目标话题测试脚本
# 用于验证新的 ArmTarget 消息类型和话题架构

echo "=========================================="
echo "ROS2 机械臂目标话题测试脚本"
echo "=========================================="
echo ""

# 加载 ROS2 环境
echo "1. 加载 ROS2 环境..."
cd /home/wx/servicerobot/ihb_qt_ros
source install/setup.bash
echo "✓ ROS2 环境已加载"
echo ""

# 检查消息类型定义
echo "2. 检查 ArmTarget 消息类型定义..."
echo "-------------------------------------------"
ros2 interface show ihb_msgs/msg/ArmTarget
echo "-------------------------------------------"
echo ""

# 列出所有话题
echo "3. 列出当前所有话题..."
echo "-------------------------------------------"
ros2 topic list
echo "-------------------------------------------"
echo ""

# 检查左臂话题
echo "4. 检查左臂话题信息..."
echo "-------------------------------------------"
echo "话题: /left_arm/target"
ros2 topic info /left_arm/target 2>/dev/null || echo "话题尚未创建 (需要运行应用程序)"
echo "-------------------------------------------"
echo ""

# 检查右臂话题
echo "5. 检查右臂话题信息..."
echo "-------------------------------------------"
echo "话题: /right_arm/target"
ros2 topic info /right_arm/target 2>/dev/null || echo "话题尚未创建 (需要运行应用程序)"
echo "-------------------------------------------"
echo ""

# 提供监听命令
echo "6. 话题监听命令 (在应用运行后使用):"
echo "-------------------------------------------"
echo "监听左臂话题:"
echo "  ros2 topic echo /left_arm/target"
echo ""
echo "监听右臂话题:"
echo "  ros2 topic echo /right_arm/target"
echo ""
echo "监听步进电机话题:"
echo "  ros2 topic echo /stepper_motor/target"
echo "-------------------------------------------"
echo ""

# 提供发布测试命令
echo "7. 手动发布测试命令示例:"
echo "-------------------------------------------"
echo "发布左臂测试消息:"
echo '  ros2 topic pub /left_arm/target ihb_msgs/msg/ArmTarget "{'
echo '    cmd: 1,'
echo '    servo_joints: ['
echo '      {id: 1, cmd: 3, angle: 0.0, speed_limit: 1000},'
echo '      {id: 2, cmd: 3, angle: 0.0, speed_limit: 1000}'
echo '    ],'
echo '    muscle_joints: ['
echo '      {id: 0, cmd: 3, angle: [0.0, 0.0], pressure: [0, 0, 0, 0]},'
echo '      {id: 1, cmd: 3, angle: [0.0, 0.0], pressure: [0, 0, 0, 0]},'
echo '      {id: 2, cmd: 3, angle: [0.0, 0.0], pressure: [0, 0, 0, 0]},'
echo '      {id: 3, cmd: 3, angle: [0.0, 0.0], pressure: [0, 0, 0, 0]}'
echo '    ]'
echo '  }"'
echo "-------------------------------------------"
echo ""

echo "=========================================="
echo "测试脚本完成"
echo "=========================================="
echo ""
echo "下一步:"
echo "1. 运行应用程序: ros2 run ihb_qt_ros ihb_qt_ros"
echo "2. 在另一个终端中监听话题以验证消息发布"
echo "3. 使用应用程序的 GUI 发送左臂/右臂命令"
echo ""

