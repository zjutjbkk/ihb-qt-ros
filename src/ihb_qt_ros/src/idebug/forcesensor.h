#ifndef FORCESENSOR_H
#define FORCESENSOR_H

#include <QWidget>
#include "ihb_msgs/msg/force_sensor_status.hpp"
namespace Ui {
class ForceSensor;
}

class ForceSensor : public QWidget
{
  Q_OBJECT

public:
  explicit ForceSensor(QWidget *parent = nullptr,int id = 0);
  void update(ihb_msgs::msg::ForceSensorStatus msg);
  ~ForceSensor();

private:
  Ui::ForceSensor *ui;
  int _id;
};

#endif // FORCESENSOR_H
