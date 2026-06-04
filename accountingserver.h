#ifndef ACCOUNTINGSERVER_H
#define ACCOUNTINGSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <QStringList>
#include <QMutex>      // 添加这行
#include <QUuid>       // 添加这行
#include <QObject>

struct UserInfo {
    QString userId;
    QString password;
    QString userName;
    QStringList bookIds;
    bool isOnline;
};

struct BookInfoNet {
    QString bookId;
    QString bookName;
    QString inviteCode;
    QString createTime;
    QString creatorId;
    QStringList memberIds;
    QString remark;
};

class AccountingServer : public QTcpServer
{
    Q_OBJECT

public:
    static AccountingServer* getInstance();
    ~AccountingServer();

    bool startServer(int port);
    void stopServer();

signals:
    void serverStarted();
    void serverStopped();
    void clientDisconnected(const QString& userId);  // 添加这个信号
    void newBookCreated(const QString& bookName, const QString& creatorId);
    void userJoined(const QString& userId, const QString& bookId);
    void newRecordAdded(const QString& bookId, const QJsonObject& record);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onReadyRead();
    void onClientError(QAbstractSocket::SocketError error);

private:
    AccountingServer(QObject *parent = nullptr);

    void processMessage(QTcpSocket* client, const QJsonObject& request);
    void sendResponse(QTcpSocket* client, const QJsonObject& response);
    void notifyBookMembers(const QString& bookId, const QJsonObject& notification);

    void handleRegister(QTcpSocket* client, const QJsonObject& request);
    void handleLogin(QTcpSocket* client, const QJsonObject& request);
    void handleCreateBook(QTcpSocket* client, const QJsonObject& request);
    void handleJoinBook(QTcpSocket* client, const QJsonObject& request);
    void handleGetBooks(QTcpSocket* client, const QJsonObject& request);
    void handleGetMembers(QTcpSocket* client, const QJsonObject& request);
    void handleAddRecord(QTcpSocket* client, const QJsonObject& request);
    void handleSyncData(QTcpSocket* client, const QJsonObject& request);
    void handleLeaveBook(QTcpSocket* client, const QJsonObject& request);

    void saveDataToFile();
    void loadDataFromFile();

    QString generateInviteCode();
    bool validateUser(const QString& userId, const QString& password);
    void addUser(const QString& userId, const QString& password, const QString& userName);

private:
    static AccountingServer* m_instance;

    QHash<QTcpSocket*, QString> m_clients;
    QHash<QString, UserInfo> m_users;
    QHash<QString, BookInfoNet> m_books;

    QString m_dataFile;
    QMutex m_dataMutex;
};

#endif // ACCOUNTINGSERVER_H