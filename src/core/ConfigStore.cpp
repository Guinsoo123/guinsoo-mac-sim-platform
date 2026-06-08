#include "core/ConfigStore.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>

namespace {

bool writeJson(const QString &path, const QJsonObject &object, QString *errorMessage)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return true;
}

QJsonObject readJson(const QString &path, QString *errorMessage)
{
    QFile file(path);
    if (!file.exists()) {
        return {};
    }
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return {};
    }
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    return document.object();
}

QString safeProjectId(QString value)
{
    value.replace("/", "_");
    value.replace(":", "_");
    value.replace(" ", "_");
    return value.isEmpty() ? "default" : value;
}

} // namespace

ConfigStore::ConfigStore(QString baseDirectory)
    : baseDirectory_(std::move(baseDirectory))
{
}

bool ConfigStore::saveGlobalConfig(const GlobalConfig &config, QString *errorMessage) const
{
    return writeJson(globalConfigPath(), toJson(config), errorMessage);
}

bool ConfigStore::saveProjectConfig(const QString &projectId, const ProjectConfig &config, QString *errorMessage) const
{
    return writeJson(projectConfigPath(projectId), toJson(config), errorMessage);
}

GlobalConfig ConfigStore::loadGlobalConfig(QString *errorMessage) const
{
    return globalConfigFromJson(readJson(globalConfigPath(), errorMessage));
}

ProjectConfig ConfigStore::loadProjectConfig(const QString &projectId, QString *errorMessage) const
{
    return projectConfigFromJson(readJson(projectConfigPath(projectId), errorMessage));
}

QString ConfigStore::globalConfigPath() const
{
    return QDir(baseDirectory_).filePath("global.json");
}

QString ConfigStore::projectConfigPath(const QString &projectId) const
{
    return QDir(baseDirectory_).filePath(QString("projects/%1.json").arg(safeProjectId(projectId)));
}
