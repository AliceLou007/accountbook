#ifndef MANAGE_H
#define MANAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QMenu>          // 添加这一行
#include <QAction>        // 添加这一行（可选，但建议）
#include "networkclient.h"

class BookDetail;
// ... 其余代码

struct BookInfo {
    QString name;
    QString createTime;
    int memberCount;
    int recordCount;
    QString remark;
    QString bookId;      // 服务器返回的账本ID
    QString inviteCode;  // 邀请码
    bool isDefault;
};

class Manage : public QWidget
{
    Q_OBJECT

public:
    explicit Manage(QWidget *parent = nullptr);
    ~Manage();

private slots:
    void onCreateBook();
    void onInviteMember();
    void onMultiBook();
    void onEditTags();
    void onSwitchBook(QTableWidgetItem *item);
    void onCurrentBookClicked();
    void onBackToManage();
    void onEditBook();
    void onDeleteBook();
    void onTableContextMenu(const QPoint &pos);

    // 网络相关槽函数
    void onConnected();
    void onDisconnected();
    void onError(const QString &error);
    void onCreateBookResult(bool success, const QString &bookId, const QString &bookName, const QString &inviteCode, const QString &message);
    void onJoinBookResult(bool success, const QString &bookName, const QString &message);
    void onGetBooksResult(const QJsonArray &books);
    void onMemberJoined(const QString &bookId, const QString &userId, const QString &userName);
    void onMemberLeft(const QString &bookId, const QString &userId, const QString &userName);
    void onNewRecord(const QString &bookId, const QJsonObject &record);

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void loadBooksFromFile();
    void saveBooksToFile();
    void saveSelectedBook();
    void loadSelectedBook();
    void loadAccountBooks();
    void updateCurrentBookLabel();
    void setupTableHover();
    void showBookDetail(int row);
    void showJoinBookDialog();
    void syncBooksFromServer();

private:
    QTableWidget *m_table;
    QPushButton *m_btnCreate;
    QPushButton *m_btnInvite;
    QPushButton *m_btnTags;
    QPushButton *m_currentBookBtn;
    QStackedWidget *m_stackedWidget;
    QWidget *m_managePage;
    BookDetail *m_detailPage;

    QList<BookInfo> m_books;
    int m_hoverRow;
    int m_selectedRow;
    int m_currentDetailRow;
    int m_currentRightClickRow;
    QString m_currentBookName;

    QString m_currentUserId;  // 当前登录用户
};

#endif // MANAGE_H