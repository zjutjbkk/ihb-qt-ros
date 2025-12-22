#include "pidparammanager.h"

// 静态成员变量定义
PidParamManager* PidParamManager::m_instance = nullptr;

PidParamManager* PidParamManager::instance()
{
    if (!m_instance) {
        m_instance = new PidParamManager();
    }
    return m_instance;
}

PidParamManager::PidParamManager(QObject *parent)
    : QObject(parent)
{
    // 设置配置文件目录
    configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                + "/ihb_robot/pid_configs/";
    
    // 确保目录存在
    QDir dir(configDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "无法创建配置目录:" << configDir;
        }
    }
    
    // 初始化为默认配置
    resetToDefaults();
}

PidParamManager::~PidParamManager()
{
}

void PidParamManager::getDefaultParams(int jointId, double &kp, double &ki, double &kd)
{
    // 参考 ihb_debug.cpp 第61-64行的默认参数
    switch(jointId) {
        case 0: kp = 30.0;  ki = 1.8;  kd = 120.0; break;  // muscle1
        case 1: kp = 60.0;  ki = 5.0;  kd = 200.0; break;  // muscle2
        case 2: kp = 300.0; ki = 10.0; kd = 150.0; break;  // muscle3
        case 3: kp = 250.0; ki = 20.0; kd = 200.0; break;  // muscle4
        case 4: kp = 30.0;  ki = 1.8;  kd = 120.0; break;  // muscle5
        case 5: kp = 60.0;  ki = 5.0;  kd = 200.0; break;  // muscle6
        case 6: kp = 300.0; ki = 10.0; kd = 150.0; break;  // muscle7
        case 7: kp = 250.0; ki = 20.0; kd = 200.0; break;  // muscle8
        default: kp = 0; ki = 0; kd = 0;
    }
}

void PidParamManager::resetToDefaults()
{
    currentConfig = PidConfig("默认配置");
    currentConfig.description = "出厂默认PID参数";
    
    for (int i = 0; i < 8; i++) {
        double kp, ki, kd;
        getDefaultParams(i, kp, ki, kd);
        currentConfig.joints[i] = PidParams(kp, ki, kd);
    }
    
    emit configChanged("默认配置");
}

bool PidParamManager::setJointParams(int jointId, double kp, double ki, double kd)
{
    if (jointId < 0 || jointId >= 8) {
        qWarning() << "无效的关节ID:" << jointId;
        return false;
    }
    
    PidParams params(kp, ki, kd);
    if (!params.isValid()) {
        qWarning() << "无效的PID参数:" << kp << ki << kd;
        return false;
    }
    
    currentConfig.joints[jointId] = params;
    emit jointParamsChanged(jointId, kp, ki, kd);
    return true;
}

void PidParamManager::getJointParams(int jointId, double &kp, double &ki, double &kd) const
{
    if (jointId < 0 || jointId >= 8) {
        qWarning() << "无效的关节ID:" << jointId;
        kp = ki = kd = 0;
        return;
    }
    
    if (currentConfig.joints.contains(jointId)) {
        const PidParams &params = currentConfig.joints[jointId];
        kp = params.kp;
        ki = params.ki;
        kd = params.kd;
    } else {
        getDefaultParams(jointId, kp, ki, kd);
    }
}

QJsonObject PidParamManager::configToJson(const PidConfig &config) const
{
    QJsonObject json;
    json["config_name"] = config.name;
    json["description"] = config.description;
    json["create_time"] = config.createTime.toString(Qt::ISODate);
    
    QJsonArray jointsArray;
    for (int i = 0; i < 8; i++) {
        if (config.joints.contains(i)) {
            QJsonObject jointObj;
            jointObj["id"] = i;
            jointObj["kp"] = config.joints[i].kp;
            jointObj["ki"] = config.joints[i].ki;
            jointObj["kd"] = config.joints[i].kd;
            jointsArray.append(jointObj);
        }
    }
    json["joints"] = jointsArray;
    
    return json;
}

PidConfig PidParamManager::jsonToConfig(const QJsonObject &json) const
{
    PidConfig config;
    config.name = json["config_name"].toString();
    config.description = json["description"].toString();
    config.createTime = QDateTime::fromString(json["create_time"].toString(), Qt::ISODate);
    
    QJsonArray jointsArray = json["joints"].toArray();
    for (const QJsonValue &value : jointsArray) {
        QJsonObject jointObj = value.toObject();
        int id = jointObj["id"].toInt();
        double kp = jointObj["kp"].toDouble();
        double ki = jointObj["ki"].toDouble();
        double kd = jointObj["kd"].toDouble();
        config.joints[id] = PidParams(kp, ki, kd);
    }
    
    return config;
}

bool PidParamManager::saveConfig(const QString &configName)
{
    // 确保目录存在
    QDir dir(configDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "无法创建配置目录:" << configDir;
            return false;
        }
    }

    // 更新配置名称
    currentConfig.name = configName;
    currentConfig.createTime = QDateTime::currentDateTime();

    // 转换为JSON
    QJsonObject json = configToJson(currentConfig);
    QJsonDocument doc(json);

    // 保存到文件
    QString filePath = configDir + configName + ".json";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件:" << filePath << file.errorString();
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "配置已保存:" << filePath;
    emit configChanged(configName);
    return true;
}

bool PidParamManager::loadConfig(const QString &configName)
{
    QString filePath = configDir + configName + ".json";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << filePath << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "无效的JSON文件:" << filePath;
        return false;
    }

    currentConfig = jsonToConfig(doc.object());

    qDebug() << "配置已加载:" << filePath;
    emit configChanged(configName);
    return true;
}

QStringList PidParamManager::getConfigList() const
{
    QDir dir(configDir);
    if (!dir.exists()) {
        return QStringList();
    }

    QStringList filters;
    filters << "*.json";
    QStringList files = dir.entryList(filters, QDir::Files);

    // 移除.json后缀
    QStringList configNames;
    for (const QString &file : files) {
        QString name = file;
        name.chop(5);  // 移除".json"
        configNames.append(name);
    }

    return configNames;
}


