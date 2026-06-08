#include "core/Diagnostics.h"

#include <QFileInfo>

namespace {

DiagnosticItem item(QString id,
                    DiagnosticLayer layer,
                    DiagnosticStatus status,
                    QString summary,
                    QString rawLog,
                    QString suggestion)
{
    return DiagnosticItem{
        std::move(id),
        layer,
        status,
        std::move(summary),
        std::move(rawLog),
        std::move(suggestion),
    };
}

bool isValidPort(int port)
{
    return port > 0 && port < 65536;
}

} // namespace

QList<DiagnosticItem> validateConfiguration(const EffectiveConfig &config)
{
    QList<DiagnosticItem> items;

    if (config.dockerExecutable.trimmed().isEmpty()) {
        items.append(item("docker-path",
                          DiagnosticLayer::MacDependency,
                          DiagnosticStatus::Fail,
                          "Mac 依赖：未配置 Docker 可执行文件路径",
                          "dockerExecutable 为空",
                          "在设置中心填写 Docker 路径，或安装 Docker Desktop 后使用默认 docker 命令。"));
    } else {
        items.append(item("docker-path",
                          DiagnosticLayer::MacDependency,
                          DiagnosticStatus::Pass,
                          "Mac 依赖：已配置 Docker 可执行文件路径",
                          config.dockerExecutable,
                          "无需处理。"));
    }

    if (config.workspacePath.trimmed().isEmpty()) {
        items.append(item("workspace-path",
                          DiagnosticLayer::RosEnvironment,
                          DiagnosticStatus::Fail,
                          "ROS 环境：未选择本地 ROS 工作区",
                          "workspacePath 为空",
                          "在设置中心选择一个本地 ROS 工作区目录，用于挂载到容器内。"));
    } else if (!QFileInfo::exists(config.workspacePath)) {
        items.append(item("workspace-path",
                          DiagnosticLayer::RosEnvironment,
                          DiagnosticStatus::Warning,
                          "ROS 环境：工作区路径当前不存在",
                          config.workspacePath,
                          "确认目录路径是否正确；如果是新项目，可以先创建该目录。"));
    } else {
        items.append(item("workspace-path",
                          DiagnosticLayer::RosEnvironment,
                          DiagnosticStatus::Pass,
                          "ROS 环境：工作区路径可访问",
                          config.workspacePath,
                          "无需处理。"));
    }

    if (!isValidPort(config.foxglovePort)) {
        items.append(item("foxglove-port",
                          DiagnosticLayer::FoxgloveBridge,
                          DiagnosticStatus::Fail,
                          "Foxglove Bridge：端口必须在 1 到 65535 之间",
                          QString::number(config.foxglovePort),
                          "在设置中心调整 Foxglove Bridge 端口，推荐使用 8765。"));
    } else {
        items.append(item("foxglove-port",
                          DiagnosticLayer::FoxgloveBridge,
                          DiagnosticStatus::Pass,
                          "Foxglove Bridge：端口配置有效",
                          QString::number(config.foxglovePort),
                          "无需处理。"));
    }

    if (config.cpuLimit <= 0) {
        items.append(item("cpu-limit",
                          DiagnosticLayer::DockerDesktop,
                          DiagnosticStatus::Fail,
                          "Docker Desktop：CPU 资源上限必须大于 0",
                          QString::number(config.cpuLimit),
                          "在设置中心配置至少 1 个 CPU。"));
    }

    if (config.memoryLimitGb <= 0) {
        items.append(item("memory-limit",
                          DiagnosticLayer::DockerDesktop,
                          DiagnosticStatus::Fail,
                          "Docker Desktop：内存资源上限必须大于 0",
                          QString::number(config.memoryLimitGb),
                          "在设置中心配置至少 1 GB 内存。"));
    }

    if (config.imageTag.trimmed().isEmpty()) {
        items.append(item("image-tag",
                          DiagnosticLayer::ImageBuild,
                          DiagnosticStatus::Fail,
                          "镜像构建：镜像标签不能为空",
                          "imageTag 为空",
                          "在设置中心填写镜像标签，例如 guinsoo/robot-sim:jazzy-arm64。"));
    }

    if (config.rosDomainId < 0 || config.rosDomainId > 232) {
        items.append(item("ros-domain-id",
                          DiagnosticLayer::RosEnvironment,
                          DiagnosticStatus::Fail,
                          "ROS 环境：ROS_DOMAIN_ID 必须在 0 到 232 之间",
                          QString::number(config.rosDomainId),
                          "在项目配置中选择合法的 ROS_DOMAIN_ID。"));
    }

    if (config.rmwImplementation.trimmed().isEmpty()) {
        items.append(item("rmw",
                          DiagnosticLayer::RosEnvironment,
                          DiagnosticStatus::Fail,
                          "ROS 环境：RMW/DDS 实现不能为空",
                          "rmwImplementation 为空",
                          "推荐使用 rmw_cyclonedds_cpp。"));
    }

    return items;
}

QString layerName(DiagnosticLayer layer)
{
    switch (layer) {
    case DiagnosticLayer::MacDependency:
        return "Mac 依赖";
    case DiagnosticLayer::DockerDesktop:
        return "Docker Desktop";
    case DiagnosticLayer::ImageBuild:
        return "镜像构建";
    case DiagnosticLayer::ContainerHealth:
        return "容器健康";
    case DiagnosticLayer::RosEnvironment:
        return "ROS 环境";
    case DiagnosticLayer::GazeboDemo:
        return "Gazebo 示例";
    case DiagnosticLayer::FoxgloveBridge:
        return "Foxglove Bridge";
    }
    return "未知";
}

QString statusName(DiagnosticStatus status)
{
    switch (status) {
    case DiagnosticStatus::Pass:
        return "通过";
    case DiagnosticStatus::Warning:
        return "警告";
    case DiagnosticStatus::Fail:
        return "失败";
    }
    return "未知";
}

