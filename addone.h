#ifndef ADDONE_H
#define ADDONE_H

#include <QDialog> // 改为继承 QDialog
#include <string>
#include <QStringList>

namespace Ui {
class AddOne;
}

// 账目结构体，用于规范存储数据
struct AccountRecord {
    std::string date;
    std::string type;
    std::string category;
    double amount;
    std::string comment;
};

class AddOne : public QDialog // 确保这里是 QDialog
{
    Q_OBJECT

public:
    explicit AddOne(QWidget *parent = nullptr);
    void setCurrentBookName(const QString &bookName);
    ~AddOne();

private slots:
    void on_buttonBox_accepted(); // 对应 OK
    void on_buttonBox_rejected(); // 对应 Cancel

private:
    Ui::AddOne *ui;
    QStringList getTopThreeRemarks();
    QString m_currentBookName;
};

#endif // ADDONE_H