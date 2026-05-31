#pragma once
#include <QWidget>
#include <QStringList>

// 定义一个结构体用来存单条账目
struct AccountItem {
    QString date;
    QString type;     // 收入/支出
    QString category; // 餐饮/购物/奖金
    double amount;
    QString remark;   // 备注
};

namespace Ui { class Record; }

class Record : public QWidget {
    Q_OBJECT
public:
    explicit Record(QWidget *parent = nullptr);
    ~Record();

private slots:
    void on_btnSortTime_clicked();     // 点击时间按钮触发
    void on_btnSortCategory_clicked(); // 点击分类按钮触发

private:
    Ui::Record *ui;
    QList<AccountItem> m_allRecords; // 缓存所有从 txt 读出来的账目
    int m_currentSortType=0;

    void loadDataFromFile();         // 读取 data.txt
    void displayRecords();           // 把排序后的数据显示到 ListWidget 上
};