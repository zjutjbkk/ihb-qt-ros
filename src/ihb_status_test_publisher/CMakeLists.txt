cmake_minimum_required(VERSION 3.8)
project(ihb_status_test_publisher)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(ihb_msgs REQUIRED)

# 添加可执行文件
add_executable(ihb_status_test_publisher src/ihb_status_test_publisher.cpp)
ament_target_dependencies(ihb_status_test_publisher rclcpp ihb_msgs)

# 安装可执行文件
install(TARGETS
  ihb_status_test_publisher
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

