#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QWebSocket>

struct FoxgloveChannel {
    int id = 0;
    QString topic;
    QString encoding;
    QString schemaName;
};

struct FoxgloveMessage {
    QString operation;
    QList<FoxgloveChannel> channels;
};

FoxgloveMessage parseFoxgloveMessage(const QByteArray &payload);
QString foxgloveEndpoint(int port);

class FoxgloveClient : public QObject {
    Q_OBJECT

public:
    explicit FoxgloveClient(QObject *parent = nullptr);

    void connectToBridge(const QUrl &url);
    void disconnectFromBridge();
    bool isConnected() const;
    QList<FoxgloveChannel> channels() const;
    QString lastError() const;

signals:
    void connected();
    void disconnected();
    void channelsChanged(const QList<FoxgloveChannel> &channels);
    void errorChanged(const QString &message);

private:
    QWebSocket socket_;
    QList<FoxgloveChannel> channels_;
    QString lastError_;
};
