#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

struct GlobalConfig {
    QString dockerExecutable = "docker";
    QString httpProxy;
    QString httpsProxy;
    QString imageTag = "guinsoo/robot-sim:jazzy-arm64";
    int cpuLimit = 4;
    int memoryLimitGb = 8;
    int foxglovePort = 8765;
    QString logLevel = "info";
};

struct ProjectConfig {
    QString workspacePath;
    QString imageTag;
    int rosDomainId = 0;
    QString rmwImplementation = "rmw_cyclonedds_cpp";
    QStringList gazeboResourcePaths;
    int foxglovePort = 0;
    QString logLevel;
};

struct EffectiveConfig {
    QString dockerExecutable;
    QString httpProxy;
    QString httpsProxy;
    QString imageTag;
    QString workspacePath;
    int cpuLimit = 0;
    int memoryLimitGb = 0;
    int rosDomainId = 0;
    QString rmwImplementation;
    QStringList gazeboResourcePaths;
    int foxglovePort = 0;
    QString logLevel;
};

EffectiveConfig mergeConfig(const GlobalConfig &global, const ProjectConfig &project);
QJsonObject toJson(const GlobalConfig &config);
QJsonObject toJson(const ProjectConfig &config);
GlobalConfig globalConfigFromJson(const QJsonObject &json);
ProjectConfig projectConfigFromJson(const QJsonObject &json);

