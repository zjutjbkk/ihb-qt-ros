#!/bin/bash

# PID调试功能快速启动脚本

echo "========================================="
echo "PID调试功能 - 快速启动"
echo "========================================="
echo ""

# 检查是否在正确的目录
if [ ! -f "install/setup.bash" ]; then
    echo "❌ 错误: 未找到 install/setup.bash"
    echo "请确保在项目根目录 /home/wx/servicerobot/ihb_qt_ros 下运行此脚本"
    exit 1
fi

# 检查编译是否成功
if [ ! -f "install/ihb_qt_ros/lib/ihb_qt_ros/ihb_qt_ros" ]; then
    echo "⚠️  警告: 可执行文件不存在，正在编译..."
    colcon build --packages-select ihb_qt_ros
    if [ $? -ne 0 ]; then
        echo "❌ 编译失败，请检查错误信息"
        exit 1
    fi
    echo "✅ 编译成功"
fi

echo "📋 功能说明:"
echo "  - 8个肌肉关节的PID参数调试"
echo "  - 配置保存/加载功能"
echo "  - 实时角度和误差曲线绘制"
echo "  - ROS通信接口（需要下位机节点）"
echo ""

echo "🎯 使用步骤:"
echo "  1. 程序启动后，点击顶部'调试'标签"
echo "  2. 点击'🔧 肌肉PID调试'子标签"
echo "  3. 修改PID参数并测试"
echo ""

echo "⚠️  注意事项:"
echo "  - ROS通信功能需要下位机节点运行"
echo "  - 实时曲线需要下位机发送IhbStatus消息"
echo "  - 配置文件保存在: ~/.config/ihb_robot/pid_configs/"
echo ""

echo "🚀 正在启动程序..."
echo "========================================="
echo ""

# 设置环境变量
source install/setup.bash

# 启动程序
ros2 run ihb_qt_ros ihb_qt_ros

