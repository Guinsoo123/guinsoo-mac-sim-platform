#pragma once

#include "core/AppConfig.h"

#include <QString>
#include <QStringList>

struct DockerCommand {
    QString program;
    QStringList arguments;
    QString description;
};

class DockerController {
public:
    explicit DockerController(EffectiveConfig config);

    DockerCommand buildImageCommand(const QString &dockerfilePath) const;
    DockerCommand runContainerCommand(const QString &containerName) const;
    DockerCommand launchDemoCommand(const QString &containerName) const;

private:
    EffectiveConfig config_;
};

