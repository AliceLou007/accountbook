#include <QApplication>
#include "widget.h"
#include "AccountingServer.h"
#include "NetworkClient.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 启动服务器
    static AccountingServer* server = AccountingServer::getInstance();
    if (!server->startServer(8888)) {
        qDebug() << "服务器启动失败:" << server->errorString();
    } else {
        qDebug() << "服务器启动成功，端口: 8888";
    }

    // 客户端连接到服务器
    NetworkClient* client = NetworkClient::getInstance();
    client->connectToServer("127.0.0.1", 8888);

    Widget w;
    w.show();

    return a.exec();
}