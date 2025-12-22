#include "liveshow.h"
#include "ui_liveshow.h"
#include <QDebug>
#include <QResizeEvent>

LiveShow::LiveShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LiveShow),
    is_receiving_(false),
    is_raw_image_subscribed_(false),
    is_detection_subscribed_(false)
{
    ui->setupUi(this);

    // 获取SignalTransmitter单例
    signalTransmitter = SignalTransmitter::instance();

    // 连接图像信号到槽函数
    bool detection_connected = connect(signalTransmitter, &SignalTransmitter::DetectionResultImageChangedSignal,
                                       this, &LiveShow::updateDetectionImage);
    bool raw_connected = connect(signalTransmitter, &SignalTransmitter::RawImageChangedSignal,
                                 this, &LiveShow::updateRawImage);

    qDebug() << "检测结果图像信号连接:" << (detection_connected ? "成功" : "失败");
    qDebug() << "原始图像信号连接:" << (raw_connected ? "成功" : "失败");

    // 初始化延迟缩放定时器
    resize_timer_ = new QTimer(this);
    resize_timer_->setSingleShot(true);  // 单次触发
    resize_timer_->setInterval(150);  // 150ms延迟
    connect(resize_timer_, &QTimer::timeout, this, &LiveShow::performDelayedResize);

    qDebug() << "LiveShow initialized";
}

LiveShow::~LiveShow()
{
    delete ui;
}

void LiveShow::on_pushButton_toggle_camera_clicked()
{
    // 这个按钮只是用来标记是否显示图像
    is_receiving_ = !is_receiving_;

    if (is_receiving_) {
        ui->pushButton_toggle_camera->setText("停止接收");
        ui->label_detection_image_info->setText("图像信息: 等待数据...");
        ui->label_raw_image_info->setText("图像信息: 等待数据...");
        qDebug() << "开始接收图像";
    } else {
        ui->pushButton_toggle_camera->setText("开始接收");
        ui->label_detection_image->setText("等待检测结果...");
        ui->label_raw_image_display->setText("等待原始图像...");
        ui->label_detection_image_info->setText("图像信息: 未接收");
        ui->label_raw_image_info->setText("图像信息: 未接收");
        qDebug() << "停止接收图像";
    }
}

void LiveShow::on_pushButton_subscribe_raw_image_clicked()
{
    if (!is_raw_image_subscribed_) {
        // 发送订阅信号
        QString topic_name = ui->lineEdit_raw_image_topic->text();
        emit signalTransmitter->SubscribeRawImageSignal(topic_name);

        ui->pushButton_subscribe_raw_image->setText("停止订阅");
        is_raw_image_subscribed_ = true;
        qDebug() << "请求订阅原始图像话题:" << topic_name;
    } else {
        // 发送取消订阅信号
        emit signalTransmitter->UnsubscribeRawImageSignal();

        ui->pushButton_subscribe_raw_image->setText("开始订阅");
        is_raw_image_subscribed_ = false;
        qDebug() << "请求取消订阅原始图像话题";
    }
}

void LiveShow::on_pushButton_subscribe_detection_clicked()
{
    if (!is_detection_subscribed_) {
        // 发送订阅信号
        QString topic_name = ui->lineEdit_detection_topic->text();
        emit signalTransmitter->SubscribeDetectionResultSignal(topic_name);

        ui->pushButton_subscribe_detection->setText("停止订阅");
        is_detection_subscribed_ = true;
        qDebug() << "请求订阅检测结果话题:" << topic_name;
    } else {
        // 发送取消订阅信号
        emit signalTransmitter->UnsubscribeDetectionResultSignal();

        ui->pushButton_subscribe_detection->setText("开始订阅");
        is_detection_subscribed_ = false;
        qDebug() << "请求取消订阅检测结果话题";
    }
}

void LiveShow::updateDetectionImage(QImage image)
{
    if (!is_receiving_) {
        return;  // 如果未开启接收，则不更新显示
    }

    // 缓存图像
    cached_detection_image_ = image;

    // 使用快速缩放立即更新显示
    updateDetectionImageDisplay(image, false);
}

void LiveShow::updateRawImage(QImage image)
{
    // 添加调试日志（每10帧打印一次，避免过于频繁）
    static int frame_count = 0;
    if (++frame_count % 1000 == 0) {
        qDebug() << "LiveShow::updateRawImage 被调用，is_receiving_:" << is_receiving_
                 << ", 图像尺寸:" << image.width() << "x" << image.height();
    }

    if (!is_receiving_) {
        return;  // 如果未开启接收，则不更新显示
    }

    // 缓存图像
    cached_raw_image_ = image;

    // 使用快速缩放立即更新显示
    updateRawImageDisplay(image, false);
}

void LiveShow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // 重启定时器，延迟执行高质量缩放
    resize_timer_->start();
}

void LiveShow::performDelayedResize()
{
    // 窗口大小稳定后，使用高质量缩放重新渲染图像
    if (!cached_detection_image_.isNull()) {
        updateDetectionImageDisplay(cached_detection_image_, true);
    }
    if (!cached_raw_image_.isNull()) {
        updateRawImageDisplay(cached_raw_image_, true);
    }
}

void LiveShow::updateDetectionImageDisplay(const QImage& image, bool force_smooth)
{
    if (image.isNull()) {
        qDebug() << "接收到空的检测结果图像";
        return;
    }

    // 缩放图像以适应label大小，保持宽高比
    // 根据force_smooth参数选择缩放质量
    QPixmap pixmap = QPixmap::fromImage(image);
    Qt::TransformationMode mode = force_smooth ? Qt::SmoothTransformation : Qt::FastTransformation;
    QPixmap scaledPixmap = pixmap.scaled(ui->label_detection_image->size(),
                                         Qt::KeepAspectRatio,
                                         mode);

    ui->label_detection_image->setPixmap(scaledPixmap);

    // 更新图像信息
    QString info = QString("图像信息: %1 x %2 | 格式: %3")
                       .arg(image.width())
                       .arg(image.height())
                       .arg(image.format());
    ui->label_detection_image_info->setText(info);
}

void LiveShow::updateRawImageDisplay(const QImage& image, bool force_smooth)
{
    if (image.isNull()) {
        qDebug() << "接收到空的原始图像";
        return;
    }

    // 移除频繁的日志输出
    // qDebug() << "updateRawImageDisplay: 开始显示原始图像，尺寸:" << image.width() << "x" << image.height()
    //          << ", force_smooth:" << force_smooth;

    // 缩放图像以适应label大小，保持宽高比
    // 根据force_smooth参数选择缩放质量
    QPixmap pixmap = QPixmap::fromImage(image);
    Qt::TransformationMode mode = force_smooth ? Qt::SmoothTransformation : Qt::FastTransformation;
    QPixmap scaledPixmap = pixmap.scaled(ui->label_raw_image_display->size(),
                                         Qt::KeepAspectRatio,
                                         mode);

    ui->label_raw_image_display->setPixmap(scaledPixmap);

    // 更新图像信息
    QString info = QString("图像信息: %1 x %2 | 格式: %3")
                       .arg(image.width())
                       .arg(image.height())
                       .arg(image.format());
    ui->label_raw_image_info->setText(info);

    // 移除频繁的日志输出
    // qDebug() << "updateRawImageDisplay: 原始图像显示完成";
}






