#ifndef LIVESHOW_H
#define LIVESHOW_H

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include "src/connect/signaltransmitter.h"

namespace Ui {
class LiveShow;
}

class LiveShow : public QWidget
{
    Q_OBJECT

public:
    explicit LiveShow(QWidget *parent = nullptr);
    ~LiveShow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_pushButton_toggle_camera_clicked();
    void on_pushButton_subscribe_raw_image_clicked();
    void on_pushButton_subscribe_detection_clicked();
    void updateDetectionImage(QImage image);
    void updateRawImage(QImage image);
    void performDelayedResize();

private:
    Ui::LiveShow *ui;
    SignalTransmitter* signalTransmitter;

    // 状态标志
    bool is_receiving_;
    bool is_raw_image_subscribed_;
    bool is_detection_subscribed_;

    // 图像缓存（用于延迟缩放）
    QImage cached_detection_image_;
    QImage cached_raw_image_;

    // 延迟缩放定时器
    QTimer* resize_timer_;

    // 辅助函数
    void updateDetectionImageDisplay(const QImage& image, bool force_smooth = false);
    void updateRawImageDisplay(const QImage& image, bool force_smooth = false);
};

#endif // LIVESHOW_H
