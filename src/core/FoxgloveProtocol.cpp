#include "core/FoxgloveProtocol.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebSocket>

FoxgloveMessage parseFoxgloveMessage(const QByteArray &payload)
{
    FoxgloveMessage message;
    const QJsonDocument document = QJsonDocument::fromJson(payload);
    if (!document.isObject()) {
        return message;
    }

    const QJsonObject root = document.object();
    message.operation = root.value("op").toString();

    const QJsonArray channels = root.value("channels").toArray();
    for (const QJsonValue &value : channels) {
        const QJsonObject object = value.toObject();
        FoxgloveChannel channel;
        channel.id = object.value("id").toInt();
        channel.topic = object.value("topic").toString();
        channel.encoding = object.value("encoding").toString();
        channel.schemaName = object.value("schemaName").toString();
        message.channels.append(channel);
    }

    return message;
}

QString foxgloveEndpoint(int port)
{
    return QString("ws://127.0.0.1:%1").arg(port);
}

FoxgloveClient::FoxgloveClient(QObject *parent)
    : QObject(parent)
{
    connect(&socket_, &QWebSocket::connected, this, [this] {
        lastError_.clear();
        emit connected();
    });
    connect(&socket_, &QWebSocket::disconnected, this, [this] {
        emit disconnected();
    });
    connect(&socket_, &QWebSocket::textMessageReceived, this, [this](const QString &payload) {
        const FoxgloveMessage message = parseFoxgloveMessage(payload.toUtf8());
        if (message.operation == "advertise" && !message.channels.isEmpty()) {
            channels_ = message.channels;
            emit channelsChanged(channels_);
        }
    });
    connect(&socket_, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        lastError_ = socket_.errorString();
        emit errorChanged(lastError_);
    });
}

void FoxgloveClient::connectToBridge(const QUrl &url)
{
    socket_.open(url);
}

void FoxgloveClient::disconnectFromBridge()
{
    socket_.close();
}

bool FoxgloveClient::isConnected() const
{
    return socket_.state() == QAbstractSocket::ConnectedState;
}

QList<FoxgloveChannel> FoxgloveClient::channels() const
{
    return channels_;
}

QString FoxgloveClient::lastError() const
{
    return lastError_;
}
