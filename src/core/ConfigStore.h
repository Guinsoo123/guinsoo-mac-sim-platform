#pragma once

#include "core/AppConfig.h"

#include <QString>

class ConfigStore {
public:
    explicit ConfigStore(QString baseDirectory);

    bool saveGlobalConfig(const GlobalConfig &config, QString *errorMessage = nullptr) const;
    bool saveProjectConfig(const QString &projectId, const ProjectConfig &config, QString *errorMessage = nullptr) const;
    GlobalConfig loadGlobalConfig(QString *errorMessage = nullptr) const;
    ProjectConfig loadProjectConfig(const QString &projectId, QString *errorMessage = nullptr) const;
    QString globalConfigPath() const;
    QString projectConfigPath(const QString &projectId) const;

private:
    QString baseDirectory_;
};

