#include "core/AppConfig.h"

#include <QJsonArray>

namespace {

QString valueOrDefault(const QString &value, const QString &fallback)
{
    return value.trimmed().isEmpty() ? fallback : value;
}

QStringList stringListFromJson(const QJsonArray &array)
{
    QStringList values;
    for (const QJsonValue &value : array) {
        if (value.isString()) {
            values.append(value.toString());
        }
    }
    return values;
}

QJsonArray stringListToJson(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value : values) {
        array.append(value);
    }
    return array;
}

} // namespace

EffectiveConfig mergeConfig(const GlobalConfig &global, const ProjectConfig &project)
{
    EffectiveConfig effective;
    effective.dockerExecutable = valueOrDefault(global.dockerExecutable, "docker");
    effective.httpProxy = global.httpProxy;
    effective.httpsProxy = global.httpsProxy;
    effective.imageTag = valueOrDefault(project.imageTag, global.imageTag);
    effective.workspacePath = project.workspacePath;
    effective.cpuLimit = global.cpuLimit;
    effective.memoryLimitGb = global.memoryLimitGb;
    effective.rosDomainId = project.rosDomainId;
    effective.rmwImplementation = valueOrDefault(project.rmwImplementation, "rmw_cyclonedds_cpp");
    effective.gazeboResourcePaths = project.gazeboResourcePaths;
    effective.foxglovePort = project.foxglovePort > 0 ? project.foxglovePort : global.foxglovePort;
    effective.logLevel = valueOrDefault(project.logLevel, global.logLevel);
    return effective;
}

QJsonObject toJson(const GlobalConfig &config)
{
    return {
        {"dockerExecutable", config.dockerExecutable},
        {"httpProxy", config.httpProxy},
        {"httpsProxy", config.httpsProxy},
        {"imageTag", config.imageTag},
        {"cpuLimit", config.cpuLimit},
        {"memoryLimitGb", config.memoryLimitGb},
        {"foxglovePort", config.foxglovePort},
        {"logLevel", config.logLevel},
    };
}

QJsonObject toJson(const ProjectConfig &config)
{
    return {
        {"workspacePath", config.workspacePath},
        {"imageTag", config.imageTag},
        {"rosDomainId", config.rosDomainId},
        {"rmwImplementation", config.rmwImplementation},
        {"gazeboResourcePaths", stringListToJson(config.gazeboResourcePaths)},
        {"foxglovePort", config.foxglovePort},
        {"logLevel", config.logLevel},
    };
}

GlobalConfig globalConfigFromJson(const QJsonObject &json)
{
    GlobalConfig config;
    config.dockerExecutable = json.value("dockerExecutable").toString(config.dockerExecutable);
    config.httpProxy = json.value("httpProxy").toString();
    config.httpsProxy = json.value("httpsProxy").toString();
    config.imageTag = json.value("imageTag").toString(config.imageTag);
    config.cpuLimit = json.value("cpuLimit").toInt(config.cpuLimit);
    config.memoryLimitGb = json.value("memoryLimitGb").toInt(config.memoryLimitGb);
    config.foxglovePort = json.value("foxglovePort").toInt(config.foxglovePort);
    config.logLevel = json.value("logLevel").toString(config.logLevel);
    return config;
}

ProjectConfig projectConfigFromJson(const QJsonObject &json)
{
    ProjectConfig config;
    config.workspacePath = json.value("workspacePath").toString();
    config.imageTag = json.value("imageTag").toString();
    config.rosDomainId = json.value("rosDomainId").toInt(config.rosDomainId);
    config.rmwImplementation = json.value("rmwImplementation").toString(config.rmwImplementation);
    config.gazeboResourcePaths = stringListFromJson(json.value("gazeboResourcePaths").toArray());
    config.foxglovePort = json.value("foxglovePort").toInt();
    config.logLevel = json.value("logLevel").toString();
    return config;
}

