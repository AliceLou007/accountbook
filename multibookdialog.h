#ifndef MULTIBOOKDIALOG_H
#define MULTIBOOKDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMap>

class MultiBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiBookDialog(const QStringList &bookNames, QWidget *parent = nullptr);
    explicit MultiBookDialog(const QStringList &bookNames, const QMap<QString, QString> &inviteCodes, QWidget *parent = nullptr);

    enum ActionType {
        Invite,      // 邀请别人
        Join         // 输入邀请码加入
    };

    ActionType getActionType() const { return m_actionType; }
    QString getSelectedBook() const { return m_selectedBook; }
    QString getInviteCode() const { return m_inviteCode; }

private slots:
    void onInviteMode();
    void onJoinMode();
    void onConfirm();
    void onBookSelected(int index);

private:
    void setupUI();
    void updateUI();
    QString generateInviteCode();

    ActionType m_actionType;
    QString m_selectedBook;
    QString m_inviteCode;
    QStringList m_bookNames;
    QMap<QString, QString> m_inviteCodes;

    QPushButton *m_inviteBtn;
    QPushButton *m_joinBtn;
    QComboBox *m_bookCombo;
    QLabel *m_inviteCodeLabel;
    QLineEdit *m_inviteCodeEdit;
    QPushButton *m_confirmBtn;
    QLabel *m_titleLabel;
    QLabel *m_hintLabel;
};

#endif // MULTIBOOKDIALOG_H
