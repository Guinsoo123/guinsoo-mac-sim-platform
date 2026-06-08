#pragma once

#include "core/AppConfig.h"

#include <QList>
#include <QString>

enum class DiagnosticLayer {
    MacDependency,
    DockerDesktop,
    ImageBuild,
    ContainerHealth,
    RosEnvironment,
    GazeboDemo,
    FoxgloveBridge
};

enum class DiagnosticStatus {
    Pass,
    Warning,
    Fail
};

struct DiagnosticItem {
    QString id;
    DiagnosticLayer layer = DiagnosticLayer::MacDependency;
    DiagnosticStatus status = DiagnosticStatus::Pass;
    QString summary;
    QString rawLog;
    QString suggestion;
};

QList<DiagnosticItem> validateConfiguration(const EffectiveConfig &config);
QString layerName(DiagnosticLayer layer);
QString statusName(DiagnosticStatus status);

