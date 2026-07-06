#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    static NetworkClient* getInstance();
    ~NetworkClient();

    void connectToServer(const QString& host, int port);
    void disconnectFromServer();
    bool waitForConnected(int timeoutMs = 3000);
    bool isConnected() const { return m_socket && m_socket->state() == QTcpSocket::ConnectedState; }
    QString currentUserId() const { return m_currentUserId; }
    void setCurrentUserId(const QString& userId) { m_currentUserId = userId; }

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);

    // 响应信号
    void registerResult(bool success, const QString& message);
    void loginResult(bool success, const QString& userName, const QString& message);
    void createBookResult(bool success, const QString& bookId, const QString& bookName, const QString& inviteCode, const QString& message);
    void joinBookResult(bool success, const QString& bookName, const QString& message);
    void getBooksResult(const QJsonArray& books);
    void getMembersResult(const QJsonArray& members);
    void addRecordResult(bool success);
    void syncDataResult(const QJsonArray& records);
    void leaveBookResult(bool success, const QString& message);

    // 实时通知信号
    void memberJoined(const QString& bookId, const QString& userId, const QString& userName);
    void memberLeft(const QString& bookId, const QString& userId, const QString& userName);
    void newRecord(const QString& bookId, const QJsonObject& record);

public slots:
    // 请求方法
    void registerUser(const QString& userId, const QString& password, const QString& userName);
    void login(const QString& userId, const QString& password);
    void createBook(const QString& bookName, const QString& remark);
    void joinBook(const QString& bookId, const QString& joinCode);
    void getBooks();
    void getMembers(const QString& bookId);
    void addRecord(const QString& bookId, const QJsonObject& record);
    void syncData(const QString& bookId, const QString& lastSyncId);
    void leaveBook(const QString& bookId);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    NetworkClient(QObject *parent = nullptr);

    void sendMessage(const QJsonObject& message);
    void processResponse(const QJsonObject& response);

private:
    static NetworkClient* m_instance;
    QTcpSocket* m_socket;
    QString m_host;
    int m_port;
    QString m_currentUserId;  // 当前登录的用户ID
};

#endif // NETWORKCLIENT_H
