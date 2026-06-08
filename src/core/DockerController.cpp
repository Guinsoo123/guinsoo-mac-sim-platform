#include "core/DockerController.h"

DockerController::DockerController(EffectiveConfig config)
    : config_(std::move(config))
{
}

DockerCommand DockerController::buildImageCommand(const QString &dockerfilePath) const
{
    return DockerCommand{
        config_.dockerExecutable,
        {"build", "--platform", "linux/arm64", "-t", config_.imageTag, "-f", dockerfilePath, "."},
        "构建 Ubuntu 24.04 ARM64 + ROS 2 Jazzy 工具链镜像",
    };
}

DockerCommand DockerController::runContainerCommand(const QString &containerName) const
{
    QStringList args = {
        "run",
        "--rm",
        "-it",
        "--name",
        containerName,
        "-p",
        QString("%1:%1").arg(config_.foxglovePort),
        "-e",
        QString("ROS_DOMAIN_ID=%1").arg(config_.rosDomainId),
        "-e",
        QString("RMW_IMPLEMENTATION=%1").arg(config_.rmwImplementation),
        "-e",
        QString("FOXGLOVE_PORT=%1").arg(config_.foxglovePort),
        "--cpus",
        QString::number(config_.cpuLimit),
        "--memory",
        QString("%1g").arg(config_.memoryLimitGb),
    };

    if (!config_.httpProxy.isEmpty()) {
        args.append({"-e", QString("HTTP_PROXY=%1").arg(config_.httpProxy)});
    }
    if (!config_.httpsProxy.isEmpty()) {
        args.append({"-e", QString("HTTPS_PROXY=%1").arg(config_.httpsProxy)});
    }
    if (!config_.workspacePath.isEmpty()) {
        args.append({"-v", QString("%1:/workspace/robot_ws").arg(config_.workspacePath)});
    }

    args.append(config_.imageTag);

    return DockerCommand{
        config_.dockerExecutable,
        args,
        "启动机器人仿真工具链容器",
    };
}

DockerCommand DockerController::launchDemoCommand(const QString &containerName) const
{
    return DockerCommand{
        config_.dockerExecutable,
        {"exec", containerName, "/opt/guinsoo/launch_demo.sh"},
        "在容器内启动 Gazebo + ROS topics 示例",
    };
}

