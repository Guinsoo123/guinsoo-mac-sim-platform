#pragma once

#include "core/AppConfig.h"
#include "core/ConfigStore.h"

#include <QMainWindow>

class QLabel;
class QLineEdit;
class QProcess;
class QTextEdit;
struct DockerCommand;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    GlobalConfig globalConfig_;
    ProjectConfig projectConfig_;
    ConfigStore configStore_;
    QLabel *statusLabel_ = nullptr;
    QLineEdit *workspaceEdit_ = nullptr;
    QLineEdit *imageTagEdit_ = nullptr;
    QLineEdit *foxglovePortEdit_ = nullptr;
    QLineEdit *rosDomainEdit_ = nullptr;
    QTextEdit *diagnosticsView_ = nullptr;
    QTextEdit *buildView_ = nullptr;
    QTextEdit *sessionView_ = nullptr;
    QTextEdit *dataView_ = nullptr;
    QProcess *activeProcess_ = nullptr;

    QWidget *createOverviewPage();
    QWidget *createSettingsPage();
    QWidget *createBuildPage();
    QWidget *createSessionPage();
    QWidget *createDataPage();

    EffectiveConfig effectiveConfig() const;
    void refreshAll();
    void applySettingsFromUi();
    void runCommand(const DockerCommand &command, QTextEdit *outputView);
};
