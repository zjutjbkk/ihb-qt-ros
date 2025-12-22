#include <chrono>
#include <memory>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "ihb_msgs/msg/ihb_status.hpp"

using namespace std::chrono_literals;

class IhbStatusTestPublisher : public rclcpp::Node
{
public:
    IhbStatusTestPublisher()
    : Node("ihb_status_test_publisher"), count_(0)
    {
        publisher_ = this->create_publisher<ihb_msgs::msg::IhbStatus>("/ihb_status", 10);
        timer_ = this->create_wall_timer(
            100ms, std::bind(&IhbStatusTestPublisher::timer_callback, this));
        
        RCLCPP_INFO(this->get_logger(), "========== IHB Status Test Publisher 已启动 ==========");
        RCLCPP_INFO(this->get_logger(), "发布话题: /ihb_status");
        RCLCPP_INFO(this->get_logger(), "发布频率: 10 Hz");
        RCLCPP_INFO(this->get_logger(), "测试数据: 8个肌肉关节，角度值为正弦波变化");
        RCLCPP_INFO(this->get_logger(), "====================================================");
    }

private:
    void timer_callback()
    {
        auto message = ihb_msgs::msg::IhbStatus();
        
        // 设置时间戳
        message.header.stamp = this->now();
        message.header.frame_id = "test";
        
        // 计算时间（用于生成正弦波）
        double time_sec = count_ * 0.1;  // 0.1秒间隔
        
        // 为8个肌肉关节生成测试数据
        for (int i = 0; i < 8; i++) {
            message.muscle_joint[i].id = i;
            
            // 生成正弦波角度数据（每个关节有不同的相位和幅度）
            // angle[0]: 基础角度 + 正弦波变化
            // angle[1]: 基础角度 + 余弦波变化
            float base_angle = 45.0f + i * 5.0f;  // 每个关节基础角度不同
            float amplitude = 20.0f;  // 振幅 ±20度
            float frequency = 0.5f + i * 0.1f;  // 每个关节频率略有不同
            
            message.muscle_joint[i].angle[0] = base_angle + amplitude * std::sin(2 * M_PI * frequency * time_sec);
            message.muscle_joint[i].angle[1] = base_angle + amplitude * std::cos(2 * M_PI * frequency * time_sec);
            
            // 设置压力值（模拟数据）
            message.muscle_joint[i].pressure[0] = 100 + i * 10;
            message.muscle_joint[i].pressure[1] = 110 + i * 10;
            message.muscle_joint[i].pressure[2] = 120 + i * 10;
            message.muscle_joint[i].pressure[3] = 130 + i * 10;
            
            // 设置刚度值
            message.muscle_joint[i].stiffness[0] = 128;
            message.muscle_joint[i].stiffness[1] = 128;
        }
        
        // 发布消息
        publisher_->publish(message);
        
        // 每10次打印一次详细信息
        if (count_ % 10 == 0) {
            RCLCPP_INFO(this->get_logger(), "已发布第 %ld 条消息 (时间: %.1f秒)", count_, time_sec);
            RCLCPP_INFO(this->get_logger(), "  关节0: angle[0]=%.1f°, angle[1]=%.1f°", 
                        message.muscle_joint[0].angle[0], 
                        message.muscle_joint[0].angle[1]);
            RCLCPP_INFO(this->get_logger(), "  关节1: angle[0]=%.1f°, angle[1]=%.1f°", 
                        message.muscle_joint[1].angle[0], 
                        message.muscle_joint[1].angle[1]);
        }
        
        count_++;
    }
    
    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<ihb_msgs::msg::IhbStatus>::SharedPtr publisher_;
    size_t count_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<IhbStatusTestPublisher>());
    rclcpp::shutdown();
    return 0;
}

