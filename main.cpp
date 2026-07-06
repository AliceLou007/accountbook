#include <QApplication>
#include "widget.h"
#include "AccountingServer.h"
#include "NetworkClient.h"
#include "logindialog.h"
#include <QCommandLineParser>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Account Book");
    parser.addHelpOption();
    parser.addOption({ "server", "仅启动多人账本服务器" });
    parser.addOption({ "local-server", "启动内置服务器（适合单机或作为主机）" });
    parser.addOption({ "host", "多人账本服务器地址", "host" });
    parser.addOption({ "port", "多人账本服务器端口", "port", "8888" });
    parser.process(a);

    QString host = parser.value("host");
    int port = parser.value("port").toInt();
    bool startLocalServer = parser.isSet("local-server") || parser.isSet("server");

    QFile configFile(QDir::currentPath() + "/server_config.json");
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonObject config = QJsonDocument::fromJson(configFile.readAll()).object();
        configFile.close();

        if (host.isEmpty()) {
            host = config["host"].toString();
        }
        if (!parser.isSet("port") && config.contains("port")) {
            port = config["port"].toInt(8888);
        }
        if (!parser.isSet("local-server") && !parser.isSet("server") && config.contains("startLocalServer")) {
            startLocalServer = config["startLocalServer"].toBool(false);
        }
    }

    if (host.isEmpty()) {
        host = "127.0.0.1";
        startLocalServer = true;
    }

    if (startLocalServer) {
        static AccountingServer* server = AccountingServer::getInstance();
        if (!server->startServer(port)) {
            qDebug() << "服务器启动失败:" << server->errorString();
        } else {
            qDebug() << "服务器启动成功，端口:" << port;
        }
    }

    if (parser.isSet("server")) {
        return a.exec();
    }

    // 客户端连接到服务器
    NetworkClient* client = NetworkClient::getInstance();
    client->connectToServer(host, port);
    client->waitForConnected(3000);

    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }
    client->setCurrentUserId(loginDialog.getUserId());

    Widget w(loginDialog.getUserId());
    w.show();

    return a.exec();
}
