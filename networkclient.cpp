#include "NetworkClient.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

NetworkClient* NetworkClient::m_instance = nullptr;

NetworkClient* NetworkClient::getInstance()
{
    if (!m_instance) {
        m_instance = new NetworkClient();
    }
    return m_instance;
}

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_port(0)
{
}

NetworkClient::~NetworkClient()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        delete m_socket;
    }
}

void NetworkClient::connectToServer(const QString& host, int port)
{
    if (m_socket) {
        disconnectFromServer();
    }

    m_host = host;
    m_port = port;
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &NetworkClient::onError);

    m_socket->connectToHost(host, port);
}

void NetworkClient::disconnectFromServer()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
    }
}

bool NetworkClient::waitForConnected(int timeoutMs)
{
    return m_socket && (m_socket->state() == QTcpSocket::ConnectedState || m_socket->waitForConnected(timeoutMs));
}

void NetworkClient::onConnected()
{
    qDebug() << "已连接到服务器";
    emit connected();
}

void NetworkClient::onDisconnected()
{
    qDebug() << "已断开服务器连接";
    m_currentUserId.clear();
    emit disconnected();
}

void NetworkClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
        processResponse(doc.object());
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QString errorMsg = m_socket->errorString();
    qDebug() << "网络错误:" << errorMsg;
    emit errorOccurred(errorMsg);
}

void NetworkClient::sendMessage(const QJsonObject& message)
{
    if (!m_socket || m_socket->state() != QTcpSocket::ConnectedState) {
        emit errorOccurred("未连接到服务器");
        return;
    }

    QJsonDocument doc(message);
    m_socket->write(doc.toJson());
    m_socket->flush();
}

void NetworkClient::processResponse(const QJsonObject& response)
{
    QString action = response["action"].toString();

    if (action == "register_result") {
        bool success = response["success"].toBool();
        QString message = response["message"].toString();
        emit registerResult(success, message);
    }
    else if (action == "login_result") {
        bool success = response["success"].toBool();
        QString userName = response["userName"].toString();
        QString userId = response["userId"].toString();
        QString message = response["message"].toString();
        if (success) {
            m_currentUserId = userId.isEmpty() ? userName : userId;
        }
        emit loginResult(success, userName, message);
    }
    else if (action == "create_book_result") {
        bool success = response["success"].toBool();
        QString bookId = response["bookId"].toString();
        QString bookName = response["bookName"].toString();
        QString inviteCode = response["inviteCode"].toString();
        QString message = response["message"].toString();
        emit createBookResult(success, bookId, bookName, inviteCode, message);
    }
    else if (action == "join_book_result") {
        bool success = response["success"].toBool();
        QString bookName = response["bookName"].toString();
        QString message = response["message"].toString();
        emit joinBookResult(success, bookName, message);
    }
    else if (action == "get_books_result") {
        QJsonArray books = response["books"].toArray();
        emit getBooksResult(books);
    }
    else if (action == "get_members_result") {
        QJsonArray members = response["members"].toArray();
        emit getMembersResult(members);
    }
    else if (action == "add_record_result") {
        bool success = response["success"].toBool();
        emit addRecordResult(success);
    }
    else if (action == "sync_data_result") {
        QJsonArray records = response["records"].toArray();
        emit syncDataResult(records);
    }
    else if (action == "leave_book_result") {
        bool success = response["success"].toBool();
        QString message = response["message"].toString();
        emit leaveBookResult(success, message);
    }
    else if (action == "member_joined") {
        QString bookId = response["bookId"].toString();
        QString userId = response["userId"].toString();
        QString userName = response["userName"].toString();
        emit memberJoined(bookId, userId, userName);
    }
    else if (action == "member_left") {
        QString bookId = response["bookId"].toString();
        QString userId = response["userId"].toString();
        QString userName = response["userName"].toString();
        emit memberLeft(bookId, userId, userName);
    }
    else if (action == "new_record") {
        QString bookId = response["bookId"].toString();
        QJsonObject record = response["record"].toObject();
        emit newRecord(bookId, record);
    }
    else if (action == "error") {
        QString message = response["message"].toString();
        emit errorOccurred(message);
    }
}

void NetworkClient::registerUser(const QString& userId, const QString& password, const QString& userName)
{
    QJsonObject request;
    request["action"] = "register";
    request["userId"] = userId;
    request["password"] = password;
    request["userName"] = userName;
    sendMessage(request);
}

void NetworkClient::login(const QString& userId, const QString& password)
{
    QJsonObject request;
    request["action"] = "login";
    request["userId"] = userId;
    request["password"] = password;
    sendMessage(request);
}

void NetworkClient::createBook(const QString& bookName, const QString& remark)
{
    QJsonObject request;
    request["action"] = "create_book";
    request["bookName"] = bookName;
    request["remark"] = remark;
    sendMessage(request);
}

void NetworkClient::joinBook(const QString& bookId, const QString& joinCode)
{
    QJsonObject request;
    request["action"] = "join_book";
    request["bookId"] = bookId;
    request["joinCode"] = joinCode;
    sendMessage(request);
}

void NetworkClient::getBooks()
{
    QJsonObject request;
    request["action"] = "get_books";
    sendMessage(request);
}

void NetworkClient::getMembers(const QString& bookId)
{
    QJsonObject request;
    request["action"] = "get_members";
    request["bookId"] = bookId;
    sendMessage(request);
}

void NetworkClient::addRecord(const QString& bookId, const QJsonObject& record)
{
    QJsonObject request;
    request["action"] = "add_record";
    request["bookId"] = bookId;
    request["record"] = record;
    sendMessage(request);
}

void NetworkClient::syncData(const QString& bookId, const QString& lastSyncId)
{
    QJsonObject request;
    request["action"] = "sync_data";
    request["bookId"] = bookId;
    request["lastSyncId"] = lastSyncId;
    sendMessage(request);
}

void NetworkClient::leaveBook(const QString& bookId)
{
    QJsonObject request;
    request["action"] = "leave_book";
    request["bookId"] = bookId;
    sendMessage(request);
}
