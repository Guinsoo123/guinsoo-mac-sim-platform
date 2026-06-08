#include "core/AppConfig.h"
#include "core/ConfigStore.h"
#include "core/Diagnostics.h"
#include "core/DockerController.h"
#include "core/FoxgloveProtocol.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QString>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {

void require(bool condition, const char *message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testProjectConfigOverridesGlobalDefaults()
{
    GlobalConfig global;
    global.dockerExecutable = "/usr/local/bin/docker";
    global.httpProxy = "http://127.0.0.1:7890";
    global.imageTag = "guinsoo/robot-sim:jazzy";
    global.cpuLimit = 4;
    global.memoryLimitGb = 8;
    global.foxglovePort = 8765;
    global.logLevel = "info";

    ProjectConfig project;
    project.workspacePath = "/Users/test/robot_ws";
    project.imageTag = "guinsoo/custom:dev";
    project.rosDomainId = 42;
    project.rmwImplementation = "rmw_cyclonedds_cpp";
    project.gazeboResourcePaths = {"/Users/test/assets/models"};
    project.foxglovePort = 9001;
    project.logLevel = "debug";

    const EffectiveConfig effective = mergeConfig(global, project);

    require(effective.dockerExecutable == "/usr/local/bin/docker", "全局 Docker 路径应该保留");
    require(effective.httpProxy == "http://127.0.0.1:7890", "全局代理应该保留");
    require(effective.workspacePath == "/Users/test/robot_ws", "项目工作区应该生效");
    require(effective.imageTag == "guinsoo/custom:dev", "项目镜像标签应该覆盖全局默认值");
    require(effective.rosDomainId == 42, "项目 ROS_DOMAIN_ID 应该生效");
    require(effective.rmwImplementation == "rmw_cyclonedds_cpp", "项目 RMW 应该生效");
    require(effective.foxglovePort == 9001, "项目 Foxglove 端口应该覆盖全局默认值");
    require(effective.logLevel == "debug", "项目日志级别应该覆盖全局默认值");
    require(effective.gazeboResourcePaths.size() == 1, "Gazebo 资源路径应该保留");
}

void testProjectConfigRoundTripsThroughJson()
{
    ProjectConfig project;
    project.workspacePath = "/Users/test/robot_ws";
    project.imageTag = "guinsoo/custom:dev";
    project.rosDomainId = 7;
    project.rmwImplementation = "rmw_cyclonedds_cpp";
    project.gazeboResourcePaths = {"/models", "/worlds"};
    project.foxglovePort = 8766;
    project.logLevel = "debug";

    const ProjectConfig restored = projectConfigFromJson(toJson(project));

    require(restored.workspacePath == project.workspacePath, "项目工作区应该能 JSON 往返");
    require(restored.imageTag == project.imageTag, "镜像标签应该能 JSON 往返");
    require(restored.rosDomainId == project.rosDomainId, "ROS_DOMAIN_ID 应该能 JSON 往返");
    require(restored.gazeboResourcePaths.size() == 2, "Gazebo 资源路径应该能 JSON 往返");
    require(restored.foxglovePort == project.foxglovePort, "Foxglove 端口应该能 JSON 往返");
}

void testConfigStorePersistsGlobalAndProjectOverrides()
{
    QTemporaryDir directory;
    require(directory.isValid(), "测试临时目录应该可用");

    ConfigStore store(directory.path());

    GlobalConfig global;
    global.dockerExecutable = "/opt/homebrew/bin/docker";
    global.imageTag = "guinsoo/robot-sim:jazzy-arm64";
    global.foxglovePort = 8765;

    ProjectConfig project;
    project.workspacePath = "/Users/test/robot_ws";
    project.imageTag = "guinsoo/project:dev";
    project.foxglovePort = 9001;

    require(store.saveGlobalConfig(global), "应该保存全局配置");
    require(store.saveProjectConfig("robot_ws", project), "应该保存项目配置");

    const EffectiveConfig effective = mergeConfig(store.loadGlobalConfig(), store.loadProjectConfig("robot_ws"));

    require(effective.dockerExecutable == "/opt/homebrew/bin/docker", "应该加载全局 Docker 路径");
    require(effective.workspacePath == "/Users/test/robot_ws", "应该加载项目工作区");
    require(effective.imageTag == "guinsoo/project:dev", "项目镜像标签应该覆盖全局配置");
    require(effective.foxglovePort == 9001, "项目 Foxglove 端口应该覆盖全局配置");
}

void testDiagnosticsProduceChineseLayeredFailures()
{
    EffectiveConfig config;
    config.dockerExecutable = "";
    config.workspacePath = "";
    config.foxglovePort = 70000;
    config.cpuLimit = 0;
    config.memoryLimitGb = 0;

    const QList<DiagnosticItem> items = validateConfiguration(config);

    require(items.size() >= 4, "应该返回多个分层诊断项");
    require(items[0].layer == DiagnosticLayer::MacDependency, "第一个诊断层应该是 Mac 依赖");

    bool foundPortFailure = false;
    bool allChineseMessages = true;
    for (const DiagnosticItem &item : items) {
        foundPortFailure = foundPortFailure || item.id == "foxglove-port";
        allChineseMessages = allChineseMessages && item.summary.contains("：");
        require(!item.suggestion.isEmpty(), "诊断项必须包含建议动作");
    }

    require(foundPortFailure, "应该诊断 Foxglove 端口错误");
    require(allChineseMessages, "诊断摘要应该使用中文格式");
}

void testFoxgloveChannelAdvertisementIsParsed()
{
    const QByteArray payload = R"({
        "op": "advertise",
        "channels": [
            {
                "id": 1,
                "topic": "/joint_states",
                "encoding": "cdr",
                "schemaName": "sensor_msgs/msg/JointState"
            },
            {
                "id": 2,
                "topic": "/tf",
                "encoding": "cdr",
                "schemaName": "tf2_msgs/msg/TFMessage"
            }
        ]
    })";

    const FoxgloveMessage message = parseFoxgloveMessage(payload);

    require(message.operation == "advertise", "应该解析 advertise 操作");
    require(message.channels.size() == 2, "应该解析两个 channel");
    require(message.channels[0].topic == "/joint_states", "应该解析 topic 名称");
    require(message.channels[0].schemaName == "sensor_msgs/msg/JointState", "应该解析 schema 名称");
}

void testFoxgloveEndpointUsesLocalWebSocket()
{
    require(foxgloveEndpoint(8765) == "ws://127.0.0.1:8765", "Foxglove 端点应该使用本机 WebSocket");
}

void testDockerRunCommandContainsWorkspaceAndRosSettings()
{
    EffectiveConfig config;
    config.dockerExecutable = "docker";
    config.imageTag = "guinsoo/robot-sim:jazzy-arm64";
    config.workspacePath = "/Users/test/robot_ws";
    config.cpuLimit = 4;
    config.memoryLimitGb = 8;
    config.rosDomainId = 12;
    config.rmwImplementation = "rmw_cyclonedds_cpp";
    config.foxglovePort = 8765;

    const DockerController controller(config);
    const DockerCommand command = controller.runContainerCommand("guinsoo-robot-sim");
    const QString joined = command.arguments.join(" ");

    require(command.program == "docker", "Docker 命令应该使用配置的可执行文件");
    require(joined.contains("-p 8765:8765"), "启动命令应该映射 Foxglove 端口");
    require(joined.contains("ROS_DOMAIN_ID=12"), "启动命令应该包含 ROS_DOMAIN_ID");
    require(joined.contains("RMW_IMPLEMENTATION=rmw_cyclonedds_cpp"), "启动命令应该包含 RMW 实现");
    require(joined.contains("/Users/test/robot_ws:/workspace/robot_ws"), "启动命令应该挂载本地 ROS 工作区");
    require(joined.contains("--cpus 4"), "启动命令应该包含 CPU 限制");
    require(joined.contains("--memory 8g"), "启动命令应该包含内存限制");
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    try {
        testProjectConfigOverridesGlobalDefaults();
        testProjectConfigRoundTripsThroughJson();
        testConfigStorePersistsGlobalAndProjectOverrides();
        testDiagnosticsProduceChineseLayeredFailures();
        testFoxgloveChannelAdvertisementIsParsed();
        testFoxgloveEndpointUsesLocalWebSocket();
        testDockerRunCommandContainsWorkspaceAndRosSettings();
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
