cmake_minimum_required(VERSION 3.8)
project(
  ihb_qt_ros
  VERSION 0.1
  LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# don't forget to add link
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Core Xml REQUIRED)

find_package(Qt5 REQUIRED COMPONENTS Widgets SerialPort Network)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)
find_package(QCustomPlot REQUIRED)
find_package(ai_msgs REQUIRED) # CHANGE
find_package(ihb_msgs REQUIRED) # CHANGE

# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 添加头文件
file(GLOB_RECURSE HEADERS "src/*.h")

# 添加UI文件
file(GLOB_RECURSE FORMS "src/*.ui")

set(RCC_FILES src/files.qrc)
set(LOGO_FILES src/logo.rc)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${MOC_SOURCES}
                               ${UI_HEADERS} ${RCC_FILES} ${LOGO_FILES})

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets
                      Qt5::SerialPort Qt5::Network Qt5::Xml)

ament_target_dependencies(
  ${PROJECT_NAME}
  rclcpp
  std_msgs
  ai_msgs
  sensor_msgs
  cv_bridge
  OpenCV
  QCustomPlot
  ihb_msgs
  )

# 确保在合适的位置添加QCustomPlot库依赖（如果不存在）
if(NOT TARGET qcustomplot)
  message(STATUS "QCustomPlot库未定义，尝试查找并链接...")
  
  # 尝试查找QCustomPlot库
  find_library(QCUSTOMPLOT_LIBRARY NAMES qcustomplot)
  
  if(QCUSTOMPLOT_LIBRARY)
    message(STATUS "已找到QCustomPlot库: ${QCUSTOMPLOT_LIBRARY}")
  else()
    message(WARNING "未找到QCustomPlot库，请确保安装了QCustomPlot或正确设置了查找路径")
    # 使用系统安装的库或手动编译
    # 可以添加自定义的查找路径：
    # set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/路径/到/qcustomplot")
  endif()
endif()

# 确保链接QCustomPlot库到目标
target_link_libraries(${PROJECT_NAME}
  # ... 现有库 ...
  ${QCUSTOMPLOT_LIBRARY}
)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights comment the
  # line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo) comment the line
  # when this package is in a git repo and when a copyright and license is added
  # to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

include(GNUInstallDirs)
install(
  TARGETS ihb_qt_ros
  DESTINATION lib/${PROJECT_NAME} # LIBRARY DESTINATION
                                          # ${CMAKE_INSTALL_LIBDIR}
          # RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

ament_package()
