#include "AccountingServer.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QRandomGenerator>
#include <QMutexLocker>
#include <QUuid>

AccountingServer* AccountingServer::m_instance = nullptr;

AccountingServer* AccountingServer::getInstance()
{
    if (!m_instance) {
        m_instance = new AccountingServer();
    }
    return m_instance;
}

AccountingServer::AccountingServer(QObject *parent)
    : QTcpServer(parent)
    , m_dataFile(QDir::currentPath() + "/server_data.json")
{
    loadDataFromFile();
}

AccountingServer::~AccountingServer()
{
    saveDataToFile();
}

bool AccountingServer::startServer(int port)
{
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "服务器启动失败:" << errorString();
        return false;
    }
    qDebug() << "服务器启动成功，端口:" << port;
    emit serverStarted();
    return true;
}

void AccountingServer::stopServer()
{
    close();
    for (QTcpSocket* socket : m_clients.keys()) {
        socket->disconnectFromHost();
    }
    emit serverStopped();
}

void AccountingServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::connected, this, &AccountingServer::onClientConnected);
    connect(socket, &QTcpSocket::disconnected, this, &AccountingServer::onClientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &AccountingServer::onReadyRead);
    connect(socket, &QAbstractSocket::errorOccurred, this, &AccountingServer::onClientError);

    m_clients[socket] = "";
}

void AccountingServer::onClientConnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        qDebug() << "客户端连接:" << socket->peerAddress().toString();
    }
}

void AccountingServer::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        QString userId = m_clients[socket];
        if (!userId.isEmpty()) {
            emit clientDisconnected(userId);
        }
        m_clients.remove(socket);
        socket->deleteLater();
    }
}

void AccountingServer::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject request = doc.object();
    processMessage(socket, request);
}

void AccountingServer::onClientError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        qDebug() << "客户端错误:" << socket->errorString();
    }
}

void AccountingServer::processMessage(QTcpSocket* client, const QJsonObject& request)
{
    QString action = request["action"].toString();

    if (action == "register") {
        handleRegister(client, request);
    } else if (action == "login") {
        handleLogin(client, request);
    } else if (action == "create_book") {
        handleCreateBook(client, request);
    } else if (action == "join_book") {
        handleJoinBook(client, request);
    } else if (action == "get_books") {
        handleGetBooks(client, request);
    } else if (action == "get_members") {
        handleGetMembers(client, request);
    } else if (action == "add_record") {
        handleAddRecord(client, request);
    } else if (action == "sync_data") {
        handleSyncData(client, request);
    } else if (action == "leave_book") {
        handleLeaveBook(client, request);
    } else {
        QJsonObject response;
        response["action"] = "error";
        response["message"] = "未知操作";
        sendResponse(client, response);
    }
}

void AccountingServer::sendResponse(QTcpSocket* client, const QJsonObject& response)
{
    QJsonDocument doc(response);
    client->write(doc.toJson());
    client->flush();
}

void AccountingServer::notifyBookMembers(const QString& bookId, const QJsonObject& notification)
{
    if (!m_books.contains(bookId)) return;

    const BookInfoNet& book = m_books[bookId];
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        QString userId = it.value();
        if (!userId.isEmpty() && book.memberIds.contains(userId)) {
            sendResponse(it.key(), notification);
        }
    }
}

void AccountingServer::handleRegister(QTcpSocket* client, const QJsonObject& request)
{
    QString userId = request["userId"].toString();
    QString password = request["password"].toString();
    QString userName = request["userName"].toString();

    QJsonObject response;
    response["action"] = "register_result";

    if (m_users.contains(userId)) {
        response["success"] = false;
        response["message"] = "用户名已存在";
    } else if (userId.isEmpty() || password.isEmpty()) {
        response["success"] = false;
        response["message"] = "用户名和密码不能为空";
    } else {
        addUser(userId, password, userName.isEmpty() ? userId : userName);
        response["success"] = true;
        response["message"] = "注册成功";
        saveDataToFile();
    }

    sendResponse(client, response);
}

void AccountingServer::handleLogin(QTcpSocket* client, const QJsonObject& request)
{
    QString userId = request["userId"].toString();
    QString password = request["password"].toString();

    QJsonObject response;
    response["action"] = "login_result";

    if (validateUser(userId, password)) {
        m_clients[client] = userId;

        response["success"] = true;
        response["message"] = "登录成功";
        response["userName"] = m_users[userId].userName;

        qDebug() << "用户登录:" << userId;
    } else {
        response["success"] = false;
        response["message"] = "用户名或密码错误";
    }

    sendResponse(client, response);
}

void AccountingServer::handleCreateBook(QTcpSocket* client, const QJsonObject& request)
{
    QString bookName = request["bookName"].toString();
    QString userId = m_clients[client];
    QString remark = request["remark"].toString();

    QJsonObject response;
    response["action"] = "create_book_result";

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "请先登录";
        sendResponse(client, response);
        return;
    }

    // 检查账本名是否已存在
    for (const BookInfoNet& book : m_books) {
        if (book.bookName == bookName) {
            response["success"] = false;
            response["message"] = "账本名称已存在";
            sendResponse(client, response);
            return;
        }
    }

    QMutexLocker locker(&m_dataMutex);

    BookInfoNet newBook;
    newBook.bookId = QString::number(QDateTime::currentMSecsSinceEpoch());
    newBook.bookName = bookName;
    newBook.inviteCode = generateInviteCode();
    newBook.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    newBook.creatorId = userId;
    newBook.memberIds.append(userId);
    newBook.remark = remark;

    m_books[newBook.bookId] = newBook;

    if (m_users.contains(userId)) {
        if (!m_users[userId].bookIds.contains(newBook.bookId)) {
            m_users[userId].bookIds.append(newBook.bookId);
        }
    }

    saveDataToFile();

    response["success"] = true;
    response["bookId"] = newBook.bookId;
    response["bookName"] = newBook.bookName;
    response["inviteCode"] = newBook.inviteCode;
    sendResponse(client, response);

    emit newBookCreated(bookName, userId);
}

void AccountingServer::handleJoinBook(QTcpSocket* client, const QJsonObject& request)
{
    QString bookId = request["bookId"].toString();
    QString userId = m_clients[client];
    QString joinCode = request["joinCode"].toString();

    QJsonObject response;
    response["action"] = "join_book_result";

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "请先登录";
        sendResponse(client, response);
        return;
    }

    if (!m_books.contains(bookId)) {
        response["success"] = false;
        response["message"] = "账本不存在";
        sendResponse(client, response);
        return;
    }

    BookInfoNet& book = m_books[bookId];

    if (book.inviteCode != joinCode) {
        response["success"] = false;
        response["message"] = "邀请码错误";
        sendResponse(client, response);
        return;
    }

    if (book.memberIds.contains(userId)) {
        response["success"] = false;
        response["message"] = "您已经是成员了";
        sendResponse(client, response);
        return;
    }

    QMutexLocker locker(&m_dataMutex);

    book.memberIds.append(userId);

    if (m_users.contains(userId)) {
        m_users[userId].bookIds.append(bookId);
    }

    saveDataToFile();

    response["success"] = true;
    response["bookId"] = bookId;
    response["bookName"] = book.bookName;
    sendResponse(client, response);

    QJsonObject notification;
    notification["action"] = "member_joined";
    notification["bookId"] = bookId;
    notification["userId"] = userId;
    notification["userName"] = m_users[userId].userName;
    notifyBookMembers(bookId, notification);

    emit userJoined(userId, bookId);
}

void AccountingServer::handleGetBooks(QTcpSocket* client, const QJsonObject& request)
{
    Q_UNUSED(request)
    QString userId = m_clients[client];

    QJsonObject response;
    response["action"] = "get_books_result";

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "请先登录";
        sendResponse(client, response);
        return;
    }

    QJsonArray booksArray;
    if (m_users.contains(userId)) {
        for (const QString& bookId : m_users[userId].bookIds) {
            if (m_books.contains(bookId)) {
                const BookInfoNet& book = m_books[bookId];
                QJsonObject bookObj;
                bookObj["bookId"] = book.bookId;
                bookObj["bookName"] = book.bookName;
                bookObj["inviteCode"] = book.inviteCode;
                bookObj["createTime"] = book.createTime;
                bookObj["creatorId"] = book.creatorId;
                bookObj["memberCount"] = book.memberIds.size();
                bookObj["remark"] = book.remark;
                booksArray.append(bookObj);
            }
        }
    }

    response["success"] = true;
    response["books"] = booksArray;
    sendResponse(client, response);
}

void AccountingServer::handleGetMembers(QTcpSocket* client, const QJsonObject& request)
{
    QString bookId = request["bookId"].toString();
    QString userId = m_clients[client];

    QJsonObject response;
    response["action"] = "get_members_result";

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "请先登录";
        sendResponse(client, response);
        return;
    }

    if (!m_books.contains(bookId)) {
        response["success"] = false;
        response["message"] = "账本不存在";
        sendResponse(client, response);
        return;
    }

    const BookInfoNet& book = m_books[bookId];
    if (!book.memberIds.contains(userId)) {
        response["success"] = false;
        response["message"] = "您不是该账本的成员";
        sendResponse(client, response);
        return;
    }

    QJsonArray membersArray;
    for (const QString& memberId : book.memberIds) {
        QJsonObject memberObj;
        memberObj["userId"] = memberId;
        if (m_users.contains(memberId)) {
            memberObj["userName"] = m_users[memberId].userName;
        } else {
            memberObj["userName"] = memberId;
        }
        membersArray.append(memberObj);
    }

    response["success"] = true;
    response["members"] = membersArray;
    sendResponse(client, response);
}

void AccountingServer::handleAddRecord(QTcpSocket* client, const QJsonObject& request)
{
    QString bookId = request["bookId"].toString();
    QString userId = m_clients[client];
    QJsonObject record = request["record"].toObject();

    QJsonObject response;
    response["action"] = "add_record_result";

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "请先登录";
        sendResponse(client, response);
        return;
    }

    if (!m_books.contains(bookId) || !m_books[bookId].memberIds.contains(userId)) {
        response["success"] = false;
        response["message"] = "无权限操作此账本";
        sendResponse(client, response);
        return;
    }

    QMutexLocker locker(&m_dataMutex);

    record["userId"] = userId;
    record["userName"] = m_users[userId].userName;
    record["timestamp"] = QString::number(QDateTime::currentMSecsSinceEpoch());
    record["syncId"] = QUuid::createUuid().toString();

    QString recordFile = QDir::currentPath() + QString("/book_%1_data.json").arg(bookId);
    QFile file(recordFile);

    QJsonArray recordsArray;
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            recordsArray = doc.array();
        }
        file.close();
    }

    recordsArray.append(record);

    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(recordsArray);
        file.write(doc.toJson());
        file.close();
    }

    response["success"] = true;
    sendResponse(client, response);

    QJsonObject notification;
    notification["action"] = "new_record";
    notification["bookId"] = bookId;
    notification["record"] = record;
    notifyBookMembers(bookId, notification);

    emit newRecordAdded(bookId, record);
}

void AccountingServer::handleSyncData(QTcpSocket* client, const QJsonObject& request)
{
    QString bookId = request["bookId"].toString();
    QString userId = m_clients[client];
    QString lastSyncId = request["lastSyncId"].toString();

    QJsonObject response;
    response["action"] = "sync_data_result";

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "请先登录";
        sendResponse(client, response);
        return;
    }

    if (!m_books.contains(bookId) || !m_books[bookId].memberIds.contains(userId)) {
        response["success"] = false;
        response["message"] = "无权限访问此账本";
        sendResponse(client, response);
        return;
    }

    QString recordFile = QDir::currentPath() + QString("/book_%1_data.json").arg(bookId);
    QFile file(recordFile);

    QJsonArray recordsArray;
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            recordsArray = doc.array();

            if (!lastSyncId.isEmpty()) {
                QJsonArray newRecords;
                bool found = false;
                for (const QJsonValue& value : recordsArray) {
                    QJsonObject record = value.toObject();
                    QString syncId = record["syncId"].toString();
                    if (syncId == lastSyncId) {
                        found = true;
                        continue;
                    }
                    if (found) {
                        newRecords.append(record);
                    }
                }
                recordsArray = newRecords;
            }
        }
        file.close();
    }

    response["success"] = true;
    response["records"] = recordsArray;
    sendResponse(client, response);
}

void AccountingServer::handleLeaveBook(QTcpSocket* client, const QJsonObject& request)
{
    QString bookId = request["bookId"].toString();
    QString userId = m_clients[client];

    QJsonObject response;
    response["action"] = "leave_book_result";

    if (userId.isEmpty()) {
        response["success"] = false;
        response["message"] = "请先登录";
        sendResponse(client, response);
        return;
    }

    if (!m_books.contains(bookId)) {
        response["success"] = false;
        response["message"] = "账本不存在";
        sendResponse(client, response);
        return;
    }

    BookInfoNet& book = m_books[bookId];
    if (!book.memberIds.contains(userId)) {
        response["success"] = false;
        response["message"] = "您不是该账本的成员";
        sendResponse(client, response);
        return;
    }

    if (book.creatorId == userId) {
        response["success"] = false;
        response["message"] = "创建者不能离开自己创建的账本";
        sendResponse(client, response);
        return;
    }

    QMutexLocker locker(&m_dataMutex);

    book.memberIds.removeAll(userId);

    if (m_users.contains(userId)) {
        m_users[userId].bookIds.removeAll(bookId);
    }

    saveDataToFile();

    response["success"] = true;
    response["message"] = "已离开账本";
    sendResponse(client, response);

    QJsonObject notification;
    notification["action"] = "member_left";
    notification["bookId"] = bookId;
    notification["userId"] = userId;
    notification["userName"] = m_users[userId].userName;
    notifyBookMembers(bookId, notification);
}

void AccountingServer::saveDataToFile()
{
    QMutexLocker locker(&m_dataMutex);

    QFile file(m_dataFile);
    if (!file.open(QIODevice::WriteOnly)) return;

    QJsonObject root;

    QJsonArray usersArray;
    for (const UserInfo& user : m_users) {
        QJsonObject userObj;
        userObj["userId"] = user.userId;
        userObj["password"] = user.password;
        userObj["userName"] = user.userName;
        QJsonArray booksArray;
        for (const QString& bookId : user.bookIds) {
            booksArray.append(bookId);
        }
        userObj["bookIds"] = booksArray;
        usersArray.append(userObj);
    }
    root["users"] = usersArray;

    QJsonArray booksArray;
    for (const BookInfoNet& book : m_books) {
        QJsonObject bookObj;
        bookObj["bookId"] = book.bookId;
        bookObj["bookName"] = book.bookName;
        bookObj["inviteCode"] = book.inviteCode;
        bookObj["createTime"] = book.createTime;
        bookObj["creatorId"] = book.creatorId;
        QJsonArray membersArray;
        for (const QString& member : book.memberIds) {
            membersArray.append(member);
        }
        bookObj["memberIds"] = membersArray;
        bookObj["remark"] = book.remark;
        booksArray.append(bookObj);
    }
    root["books"] = booksArray;

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
}

void AccountingServer::loadDataFromFile()
{
    QFile file(m_dataFile);
    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject root = doc.object();

    QJsonArray usersArray = root["users"].toArray();
    for (const QJsonValue& value : usersArray) {
        QJsonObject userObj = value.toObject();
        UserInfo user;
        user.userId = userObj["userId"].toString();
        user.password = userObj["password"].toString();
        user.userName = userObj["userName"].toString();
        user.isOnline = false;

        QJsonArray booksArray = userObj["bookIds"].toArray();
        for (const QJsonValue& bookVal : booksArray) {
            user.bookIds.append(bookVal.toString());
        }
        m_users[user.userId] = user;
    }

    QJsonArray booksArray = root["books"].toArray();
    for (const QJsonValue& value : booksArray) {
        QJsonObject bookObj = value.toObject();
        BookInfoNet book;
        book.bookId = bookObj["bookId"].toString();
        book.bookName = bookObj["bookName"].toString();
        book.inviteCode = bookObj["inviteCode"].toString();
        book.createTime = bookObj["createTime"].toString();
        book.creatorId = bookObj["creatorId"].toString();

        QJsonArray membersArray = bookObj["memberIds"].toArray();
        for (const QJsonValue& member : membersArray) {
            book.memberIds.append(member.toString());
        }
        book.remark = bookObj["remark"].toString();
        m_books[book.bookId] = book;
    }
}

QString AccountingServer::generateInviteCode()
{
    const QString characters = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QString code;
    for (int i = 0; i < 6; ++i) {
        int index = QRandomGenerator::global()->bounded(characters.length());
        code.append(characters.at(index));
    }
    return code;
}

bool AccountingServer::validateUser(const QString& userId, const QString& password)
{
    if (!m_users.contains(userId)) return false;
    return m_users[userId].password == password;
}

void AccountingServer::addUser(const QString& userId, const QString& password, const QString& userName)
{
    UserInfo newUser;
    newUser.userId = userId;
    newUser.password = password;
    newUser.userName = userName;
    newUser.isOnline = false;
    m_users[userId] = newUser;
}