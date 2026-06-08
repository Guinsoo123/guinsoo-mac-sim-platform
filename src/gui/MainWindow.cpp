#include "gui/MainWindow.h"

#include "core/Diagnostics.h"
#include "core/DockerController.h"
#include "core/FoxgloveProtocol.h"

#include <QDir>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

QString commandToText(const DockerCommand &command)
{
    return command.program + " " + command.arguments.join(" ");
}

QTextEdit *readOnlyText()
{
    auto *text = new QTextEdit;
    text->setReadOnly(true);
    return text;
}

QString configDirectory()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return path.isEmpty() ? ".guinsoo-mac-sim-platform" : path;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , configStore_(configDirectory())
{
    globalConfig_ = configStore_.loadGlobalConfig();
    projectConfig_ = configStore_.loadProjectConfig("default");

    setWindowTitle("Guinsoo Mac 机器人仿真平台");
    resize(1180, 760);

    auto *tabs = new QTabWidget;
    tabs->addTab(createOverviewPage(), "环境概览");
    tabs->addTab(createSettingsPage(), "设置中心");
    tabs->addTab(createBuildPage(), "镜像构建");
    tabs->addTab(createSessionPage(), "仿真会话");
    tabs->addTab(createDataPage(), "实时数据");
    setCentralWidget(tabs);

    refreshAll();
}

QWidget *MainWindow::createOverviewPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    statusLabel_ = new QLabel;
    diagnosticsView_ = readOnlyText();
    auto *refreshButton = new QPushButton("刷新诊断");
    QObject::connect(refreshButton, &QPushButton::clicked, this, [this] { refreshAll(); });

    layout->addWidget(new QLabel("工具链环境 MVP"));
    layout->addWidget(statusLabel_);
    layout->addWidget(refreshButton);
    layout->addWidget(diagnosticsView_);
    return page;
}

QWidget *MainWindow::createSettingsPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    auto *form = new QFormLayout;

    workspaceEdit_ = new QLineEdit;
    imageTagEdit_ = new QLineEdit(globalConfig_.imageTag);
    foxglovePortEdit_ = new QLineEdit(QString::number(globalConfig_.foxglovePort));
    rosDomainEdit_ = new QLineEdit(QString::number(projectConfig_.rosDomainId));
    auto *dockerEdit = new QLineEdit(globalConfig_.dockerExecutable);
    auto *httpProxyEdit = new QLineEdit(globalConfig_.httpProxy);
    auto *httpsProxyEdit = new QLineEdit(globalConfig_.httpsProxy);
    auto *cpuEdit = new QLineEdit(QString::number(globalConfig_.cpuLimit));
    auto *memoryEdit = new QLineEdit(QString::number(globalConfig_.memoryLimitGb));
    auto *rmwEdit = new QLineEdit(projectConfig_.rmwImplementation);
    auto *logLevelEdit = new QLineEdit(globalConfig_.logLevel);
    auto *gazeboPathsEdit = new QLineEdit(projectConfig_.gazeboResourcePaths.join(";"));
    workspaceEdit_->setText(projectConfig_.workspacePath);

    form->addRow("Docker 路径", dockerEdit);
    form->addRow("HTTP 代理", httpProxyEdit);
    form->addRow("HTTPS 代理", httpsProxyEdit);
    form->addRow("CPU 上限", cpuEdit);
    form->addRow("内存上限 GB", memoryEdit);
    form->addRow("镜像标签", imageTagEdit_);
    form->addRow("本地 ROS 工作区", workspaceEdit_);
    form->addRow("ROS_DOMAIN_ID", rosDomainEdit_);
    form->addRow("RMW/DDS", rmwEdit);
    form->addRow("Gazebo 资源路径", gazeboPathsEdit);
    form->addRow("Foxglove 端口", foxglovePortEdit_);
    form->addRow("日志级别", logLevelEdit);

    auto *saveButton = new QPushButton("应用配置");
    QObject::connect(saveButton, &QPushButton::clicked, this, [=, this] {
        globalConfig_.dockerExecutable = dockerEdit->text();
        globalConfig_.httpProxy = httpProxyEdit->text();
        globalConfig_.httpsProxy = httpsProxyEdit->text();
        globalConfig_.cpuLimit = cpuEdit->text().toInt();
        globalConfig_.memoryLimitGb = memoryEdit->text().toInt();
        globalConfig_.imageTag = imageTagEdit_->text();
        globalConfig_.foxglovePort = foxglovePortEdit_->text().toInt();
        globalConfig_.logLevel = logLevelEdit->text();
        projectConfig_.workspacePath = workspaceEdit_->text();
        projectConfig_.imageTag = imageTagEdit_->text();
        projectConfig_.rosDomainId = rosDomainEdit_->text().toInt();
        projectConfig_.rmwImplementation = rmwEdit->text();
        projectConfig_.gazeboResourcePaths = gazeboPathsEdit->text().split(";", Qt::SkipEmptyParts);
        projectConfig_.foxglovePort = foxglovePortEdit_->text().toInt();
        projectConfig_.logLevel = logLevelEdit->text();
        QString error;
        if (!configStore_.saveGlobalConfig(globalConfig_, &error)) {
            statusLabel_->setText("保存全局配置失败：" + error);
        }
        if (!configStore_.saveProjectConfig("default", projectConfig_, &error)) {
            statusLabel_->setText("保存项目配置失败：" + error);
        }
        refreshAll();
    });

    layout->addLayout(form);
    layout->addWidget(saveButton);
    layout->addStretch();
    return page;
}

QWidget *MainWindow::createBuildPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    buildView_ = readOnlyText();
    auto *refreshButton = new QPushButton("生成构建命令预览");
    auto *buildButton = new QPushButton("开始构建镜像");
    QObject::connect(refreshButton, &QPushButton::clicked, this, [this] { refreshAll(); });
    QObject::connect(buildButton, &QPushButton::clicked, this, [this] {
        runCommand(DockerController(effectiveConfig()).buildImageCommand("docker/Dockerfile"), buildView_);
    });
    layout->addWidget(new QLabel("本地构建 Ubuntu 24.04 ARM64 + ROS 2 Jazzy 工具链镜像"));
    layout->addWidget(refreshButton);
    layout->addWidget(buildButton);
    layout->addWidget(buildView_);
    return page;
}

QWidget *MainWindow::createSessionPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    sessionView_ = readOnlyText();
    auto *refreshButton = new QPushButton("生成会话命令预览");
    auto *startButton = new QPushButton("启动容器");
    auto *demoButton = new QPushButton("启动 Gazebo 示例");
    QObject::connect(refreshButton, &QPushButton::clicked, this, [this] { refreshAll(); });
    QObject::connect(startButton, &QPushButton::clicked, this, [this] {
        runCommand(DockerController(effectiveConfig()).runContainerCommand("guinsoo-robot-sim"), sessionView_);
    });
    QObject::connect(demoButton, &QPushButton::clicked, this, [this] {
        runCommand(DockerController(effectiveConfig()).launchDemoCommand("guinsoo-robot-sim"), sessionView_);
    });
    layout->addWidget(new QLabel("仿真会话控制面"));
    layout->addWidget(refreshButton);
    layout->addWidget(startButton);
    layout->addWidget(demoButton);
    layout->addWidget(sessionView_);
    return page;
}

QWidget *MainWindow::createDataPage()
{
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    dataView_ = readOnlyText();
    auto *refreshButton = new QPushButton("刷新数据端点");
    QObject::connect(refreshButton, &QPushButton::clicked, this, [this] { refreshAll(); });
    layout->addWidget(new QLabel("Foxglove Bridge 数据面"));
    layout->addWidget(refreshButton);
    layout->addWidget(dataView_);
    return page;
}

EffectiveConfig MainWindow::effectiveConfig() const
{
    return mergeConfig(globalConfig_, projectConfig_);
}

void MainWindow::refreshAll()
{
    const EffectiveConfig config = effectiveConfig();
    const QList<DiagnosticItem> diagnostics = validateConfiguration(config);

    QString diagnosticText;
    int failures = 0;
    for (const DiagnosticItem &item : diagnostics) {
        if (item.status == DiagnosticStatus::Fail) {
            failures += 1;
        }
        diagnosticText += QString("[%1][%2] %3\n原始信息：%4\n建议动作：%5\n\n")
                              .arg(layerName(item.layer), statusName(item.status), item.summary, item.rawLog, item.suggestion);
    }

    if (statusLabel_) {
        statusLabel_->setText(failures == 0 ? "当前配置可进入镜像构建阶段" : QString("当前存在 %1 个失败诊断项").arg(failures));
    }
    if (diagnosticsView_) {
        diagnosticsView_->setPlainText(diagnosticText);
    }

    const DockerController docker(config);
    if (buildView_) {
        buildView_->setPlainText(commandToText(docker.buildImageCommand("docker/Dockerfile")) + "\n\n说明：点击真实构建按钮前，请确认 Docker Desktop 已启动。");
    }
    if (sessionView_) {
        sessionView_->setPlainText(commandToText(docker.runContainerCommand("guinsoo-robot-sim")) + "\n\n" +
                                   commandToText(docker.launchDemoCommand("guinsoo-robot-sim")));
    }
    if (dataView_) {
        dataView_->setPlainText(QString("Foxglove Bridge 端点：%1\n\n核心库已包含 Qt WebSockets 客户端封装，可解析 advertise 消息并刷新 topic 列表。")
                                    .arg(foxgloveEndpoint(config.foxglovePort)));
    }
}

void MainWindow::applySettingsFromUi()
{
    refreshAll();
}

void MainWindow::runCommand(const DockerCommand &command, QTextEdit *outputView)
{
    if (!outputView) {
        return;
    }

    if (activeProcess_ && activeProcess_->state() != QProcess::NotRunning) {
        outputView->append("已有命令正在运行，请等待完成后再启动新的任务。");
        return;
    }

    activeProcess_ = new QProcess(this);
    activeProcess_->setWorkingDirectory(QDir::currentPath());
    activeProcess_->setProcessChannelMode(QProcess::MergedChannels);

    outputView->append("\n$ " + commandToText(command));
    QObject::connect(activeProcess_, &QProcess::readyReadStandardOutput, this, [this, outputView] {
        outputView->append(QString::fromUtf8(activeProcess_->readAllStandardOutput()));
    });
    QObject::connect(activeProcess_, &QProcess::finished, this, [this, outputView](int exitCode, QProcess::ExitStatus status) {
        const QString statusText = status == QProcess::NormalExit ? "正常退出" : "异常退出";
        outputView->append(QString("命令结束：%1，退出码：%2").arg(statusText).arg(exitCode));
        activeProcess_->deleteLater();
        activeProcess_ = nullptr;
    });
    QObject::connect(activeProcess_, &QProcess::errorOccurred, this, [outputView](QProcess::ProcessError error) {
        outputView->append(QString("命令启动失败：%1").arg(static_cast<int>(error)));
    });

    activeProcess_->start(command.program, command.arguments);
}
