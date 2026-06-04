#include "logindialog.h"
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_isLoginMode(true)
{
    setupUI();
    setWindowTitle("记账本 - 登录");
    setFixedSize(350, 320);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
}

void LoginDialog::setupUI()
{
    setStyleSheet("background-color: white;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // 标题
    m_titleLabel = new QLabel("登录", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #8c1515;"
        "   padding: 10px;"
        "}"
        );
    mainLayout->addWidget(m_titleLabel);

    // 用户名
    QLabel *userIdLabel = new QLabel("用户名：", this);
    userIdLabel->setStyleSheet("font-size: 14px; color: black;");
    m_userIdEdit = new QLineEdit(this);
    m_userIdEdit->setPlaceholderText("请输入用户名");
    m_userIdEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "   color: black;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #8c1515;"
        "}"
        );
    mainLayout->addWidget(userIdLabel);
    mainLayout->addWidget(m_userIdEdit);

    // 密码
    QLabel *pwdLabel = new QLabel("密码：", this);
    pwdLabel->setStyleSheet("font-size: 14px; color: black;");
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "   color: black;"
        "}"
        );
    mainLayout->addWidget(pwdLabel);
    mainLayout->addWidget(m_passwordEdit);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    m_loginBtn = new QPushButton("登录", this);
    m_registerBtn = new QPushButton("注册", this);
    m_switchBtn = new QPushButton("去注册", this);

    QString btnStyle =
        "QPushButton {"
        "   background-color: #8c1515;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 8px 20px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #a02020;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #6b1010;"
        "}";

    QString switchBtnStyle =
        "QPushButton {"
        "   background-color: #f0f0f0;"
        "   color: #333333;"
        "   border: 1px solid #cccccc;"
        "   border-radius: 5px;"
        "   padding: 8px 20px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}";

    m_loginBtn->setStyleSheet(btnStyle);
    m_registerBtn->setStyleSheet(btnStyle);
    m_switchBtn->setStyleSheet(switchBtnStyle);

    btnLayout->addWidget(m_loginBtn);
    btnLayout->addWidget(m_switchBtn);
    mainLayout->addLayout(btnLayout);

    // 连接信号
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginDialog::onRegister);
    connect(m_switchBtn, &QPushButton::clicked, this, [this]() {
        m_isLoginMode = !m_isLoginMode;
        if (m_isLoginMode) {
            m_titleLabel->setText("登录");
            m_loginBtn->setVisible(true);
            m_registerBtn->setVisible(false);
            m_switchBtn->setText("去注册");
            m_userIdEdit->clear();
            m_passwordEdit->clear();
        } else {
            m_titleLabel->setText("注册");
            m_loginBtn->setVisible(false);
            m_registerBtn->setVisible(true);
            m_switchBtn->setText("返回登录");
            m_userIdEdit->clear();
            m_passwordEdit->clear();
        }
    });

    m_loginBtn->setVisible(true);
    m_registerBtn->setVisible(false);
}

bool LoginDialog::checkUserExists(const QString &userId)
{
    QString filePath = QDir::currentPath() + "/users.json";
    QFile file(filePath);
    if (!file.exists()) return false;

    if (!file.open(QIODevice::ReadOnly)) return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return false;

    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        if (obj["userId"].toString() == userId) {
            return true;
        }
    }
    return false;
}

bool LoginDialog::checkPassword(const QString &userId, const QString &password)
{
    QString filePath = QDir::currentPath() + "/users.json";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return false;

    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        if (obj["userId"].toString() == userId && obj["password"].toString() == password) {
            return true;
        }
    }
    return false;
}

void LoginDialog::saveUser(const QString &userId, const QString &password)
{
    QString filePath = QDir::currentPath() + "/users.json";
    QFile file(filePath);

    QJsonArray array;
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            array = doc.array();
        }
    }

    QJsonObject newUser;
    newUser["userId"] = userId;
    newUser["password"] = password;
    newUser["userName"] = userId;

    array.append(newUser);

    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(array);
        file.write(doc.toJson());
        file.close();
    }
}

void LoginDialog::showMessageBox(const QString &title, const QString &text, QMessageBox::Icon icon)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(icon);
    msgBox.setStyleSheet(
        "QMessageBox { background-color: white; }"
        "QLabel { color: black; font-size: 14px; }"
        "QPushButton { color: black; padding: 5px 15px; }"
        );
    msgBox.exec();
}

void LoginDialog::onLogin()
{
    QString userId = m_userIdEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();

    if (userId.isEmpty() || password.isEmpty()) {
        showMessageBox("提示", "请输入用户名和密码", QMessageBox::Warning);
        return;
    }

    if (!checkUserExists(userId)) {
        showMessageBox("登录失败", "用户不存在，请先注册", QMessageBox::Warning);
        return;
    }

    if (!checkPassword(userId, password)) {
        showMessageBox("登录失败", "密码错误", QMessageBox::Warning);
        return;
    }

    accept();
}

void LoginDialog::onRegister()
{
    QString userId = m_userIdEdit->text().trimmed();
    QString password = m_passwordEdit->text().trimmed();

    if (userId.isEmpty() || password.isEmpty()) {
        showMessageBox("提示", "请输入用户名和密码", QMessageBox::Warning);
        return;
    }

    if (checkUserExists(userId)) {
        showMessageBox("注册失败", "用户名已存在", QMessageBox::Warning);
        return;
    }

    saveUser(userId, password);
    showMessageBox("注册成功", "注册成功！请登录", QMessageBox::Information);

    // 切换回登录模式
    m_isLoginMode = true;
    m_titleLabel->setText("登录");
    m_loginBtn->setVisible(true);
    m_registerBtn->setVisible(false);
    m_switchBtn->setText("去注册");
    m_userIdEdit->clear();
    m_passwordEdit->clear();
}