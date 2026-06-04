#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString getUserId() const { return m_userIdEdit->text().trimmed(); }

private slots:
    void onLogin();
    void onRegister();

private:
    void setupUI();
    bool checkUserExists(const QString &userId);
    bool checkPassword(const QString &userId, const QString &password);
    void saveUser(const QString &userId, const QString &password);
    void showMessageBox(const QString &title, const QString &text, QMessageBox::Icon icon);

    QLineEdit *m_userIdEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;
    QPushButton *m_switchBtn;
    QLabel *m_titleLabel;

    bool m_isLoginMode;
};

#endif // LOGINDIALOG_H