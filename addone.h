#ifndef ADDONE_H
#define ADDONE_H

#include <QDialog>
#include <string>
#include <QStringList>

namespace Ui {
class AddOne;
}

// 账目结构体，用于规范存储数据
struct AccountRecord {
    std::string bookName;
    std::string date;
    std::string type;
    std::string category;
    double amount;
    std::string comment;
};

class AddOne : public QDialog
{
    Q_OBJECT

public:
    explicit AddOne(QWidget *parent = nullptr);
    void setCurrentBookName(const QString &bookName);
    ~AddOne();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void onTypeChanged(const QString &type);  // 添加：类型改变时的处理

private:
    QStringList getTopThreeRemarks();
    void loadBookNames();
    void loadSelectedBook();
    void updateBookRecordCount(const QString &bookName);
    void loadCategoriesFromTags();  // 添加：从 tags.json 加载分类标签

private:
    Ui::AddOne *ui;
    QString m_currentBookName;
    QStringList m_expenseTags;  // 添加：支出标签列表
    QStringList m_incomeTags;   // 添加：收入标签列表
};

#endif // ADDONE_H