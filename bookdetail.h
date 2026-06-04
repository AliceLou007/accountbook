#ifndef BOOKDETAIL_H
#define BOOKDETAIL_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>

struct RecordItem {
    QString date;
    QString type;
    QString category;
    double amount;
    QString remark;
};

class BookDetail : public QWidget
{
    Q_OBJECT

public:
    explicit BookDetail(const QString &bookName, const QString &remark, const QStringList &members, QWidget *parent = nullptr);
    ~BookDetail();

signals:
    void backToManage();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onBack();

private:
    void setupUI();
    void loadRecords();
    void updateStats(double totalIncome, double totalOutcome);

private:
    QString m_bookName;
    QString m_remark;
    QStringList m_members;

    QLabel *m_nameLabel;
    QLabel *m_remarkLabel;
    QLabel *m_membersLabel;
    QLabel *m_incomeLabel;
    QLabel *m_outcomeLabel;
    QLabel *m_balanceLabel;
    QTableWidget *m_table;
    QPushButton *m_btnBack;
};

#endif // BOOKDETAIL_H