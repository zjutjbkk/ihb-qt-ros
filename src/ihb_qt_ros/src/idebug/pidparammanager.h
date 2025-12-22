#ifndef PIDPARAMMANAGER_H
#define PIDPARAMMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

// 单个关节的PID参数结构
struct PidParams {
    double kp;
    double ki;
    double kd;
    
    PidParams(double p = 0, double i = 0, double d = 0) 
        : kp(p), ki(i), kd(d) {}
    
    bool isValid() const {
        return kp >= 0 && kp <= 1000 &&
               ki >= 0 && ki <= 100 &&
               kd >= 0 && kd <= 1000;
    }
};

// 一套完整的配置(8个关节)
struct PidConfig {
    QString name;
    QString description;
    QMap<int, PidParams> joints;
    QDateTime createTime;
    
    PidConfig(const QString &n = "默认配置") 
        : name(n), createTime(QDateTime::currentDateTime()) {}
};

// PID参数管理器(单例模式)
class PidParamManager : public QObject
{
    Q_OBJECT
    
public:
    // 获取单例实例
    static PidParamManager* instance();
    
    // 配置管理
    bool saveConfig(const QString &configName);
    bool loadConfig(const QString &configName);
    QStringList getConfigList() const;
    PidConfig getCurrentConfig() const { return currentConfig; }
    
    // 参数操作
    bool setJointParams(int jointId, double kp, double ki, double kd);
    void getJointParams(int jointId, double &kp, double &ki, double &kd) const;
    
    // 默认值管理
    static void getDefaultParams(int jointId, double &kp, double &ki, double &kd);
    void resetToDefaults();
    
signals:
    void configChanged(const QString &configName);
    void jointParamsChanged(int jointId, double kp, double ki, double kd);
    
private:
    explicit PidParamManager(QObject *parent = nullptr);
    ~PidParamManager();
    
    // 禁止拷贝
    PidParamManager(const PidParamManager&) = delete;
    PidParamManager& operator=(const PidParamManager&) = delete;
    
    // JSON序列化
    QJsonObject configToJson(const PidConfig &config) const;
    PidConfig jsonToConfig(const QJsonObject &json) const;
    
    // 成员变量
    static PidParamManager *m_instance;
    PidConfig currentConfig;
    QString configDir;
};

#endif // PIDPARAMMANAGER_H

