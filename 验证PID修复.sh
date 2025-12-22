#!/bin/bash

# PID调试界面修复 - 快速验证脚本

echo "========================================="
echo "PID调试界面修复 - 快速验证脚本"
echo "========================================="
echo ""

WORKSPACE_DIR="/home/wx/servicerobot/ihb_qt_ros"
cd "$WORKSPACE_DIR" || exit 1

echo "📁 工作目录: $WORKSPACE_DIR"
echo ""

# 检查关键修改
echo "检查关键修改..."
echo "-------------------------------------------"

if grep -q "message.kp = muscle_joint_target.kp" "src/ihb_qt_ros/src/connect/qtnodesettings.cpp"; then
    echo "✅ qtnodesettings.cpp: kp 字段已添加"
else
    echo "❌ qtnodesettings.cpp: kp 字段缺失"
fi

if grep -q "message.ki = muscle_joint_target.ki" "src/ihb_qt_ros/src/connect/qtnodesettings.cpp"; then
    echo "✅ qtnodesettings.cpp: ki 字段已添加"
else
    echo "❌ qtnodesettings.cpp: ki 字段缺失"
fi

if grep -q "message.kd = muscle_joint_target.kd" "src/ihb_qt_ros/src/connect/qtnodesettings.cpp"; then
    echo "✅ qtnodesettings.cpp: kd 字段已添加"
else
    echo "❌ qtnodesettings.cpp: kd 字段缺失"
fi

if grep -q "doubleSpinBox_target_angle0" "src/ihb_qt_ros/src/idebug/pidjointwidget.ui"; then
    echo "✅ pidjointwidget.ui: 目标角度0输入框已添加"
else
    echo "❌ pidjointwidget.ui: 目标角度0输入框缺失"
fi

if grep -q "doubleSpinBox_target_angle1" "src/ihb_qt_ros/src/idebug/pidjointwidget.ui"; then
    echo "✅ pidjointwidget.ui: 目标角度1输入框已添加"
else
    echo "❌ pidjointwidget.ui: 目标角度1输入框缺失"
fi

echo ""
echo "编译项目..."
echo "-------------------------------------------"

colcon build --packages-select ihb_qt_ros

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ 编译成功！"
    echo ""
    echo "📝 测试步骤："
    echo ""
    echo "【终端1 - 监控话题】"
    echo "  source install/setup.bash"
    echo "  ros2 topic echo /ihb_muscle_joint_target"
    echo ""
    echo "【终端2 - 启动上位机】"
    echo "  source install/setup.bash"
    echo "  ros2 run ihb_qt_ros ihb_qt_ros"
    echo ""
    echo "详细测试指南: PID调试界面修复-测试指南.md"
else
    echo ""
    echo "❌ 编译失败，请检查错误信息"
fi

echo ""
echo "========================================="

