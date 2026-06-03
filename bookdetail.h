#ifndef BOOKDETAIL_H
#define BOOKDETAIL_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QShowEvent>

class BookDetail : public QWidget
{
    Q_OBJECT

public:
    explicit BookDetail(const QString &bookName, const QString &remark, const QStringList &members, QWidget *parent = nullptr);
    ~BookDetail();

signals:
    void backToManage();  // 返回管理界面的信号

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onBack();

private:
    void setupUI();
    void loadRecords();

private:
    QString m_bookName;
    QString m_remark;
    QStringList m_members;
    QTableWidget *m_table;
    QPushButton *m_btnBack;
    QLabel *m_nameLabel;
    QLabel *m_remarkLabel;
    QLabel *m_membersLabel;
};

#endif // BOOKDETAIL_H