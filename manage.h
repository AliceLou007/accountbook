#ifndef MANAGE_H
#define MANAGE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QList>
#include <QMenu>
#include <QStackedWidget>
#include <QLabel>

struct BookInfo {
    QString name;
    QString createTime;
    int memberCount;
    int recordCount;
    QString remark;
};

class BookDetail;

class Manage : public QWidget
{
    Q_OBJECT

public:
    Manage(QWidget *parent = nullptr);
    ~Manage();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onCreateBook();
    void onInviteMember();
    void onEditTags();
    void onSwitchBook(QTableWidgetItem *item);
    void onTableContextMenu(const QPoint &pos);
    void onEditBook();
    void onDeleteBook();
    void onBackToManage();
    void onCurrentBookClicked();

private:
    void setupUI();
    void loadAccountBooks();
    void setupTableHover();
    void saveBooksToFile();
    void loadBooksFromFile();
    void saveSelectedBook();      // 保存选中的账本
    void loadSelectedBook();      // 加载选中的账本
    void showBookDetail(int row);
    void updateCurrentBookLabel();

private:
    QTableWidget *m_table;
    QPushButton *m_btnCreate;
    QPushButton *m_btnInvite;
    QPushButton *m_btnTags;
    QPushButton *m_currentBookBtn;
    int m_hoverRow;
    int m_selectedRow;
    QList<BookInfo> m_books;
    int m_currentRightClickRow;

    QStackedWidget *m_stackedWidget;
    QWidget *m_managePage;
    BookDetail *m_detailPage;
    int m_currentDetailRow;
    QString m_currentBookName;
};

#endif // MANAGE_H