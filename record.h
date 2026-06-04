#ifndef RECORD_H
#define RECORD_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>
#include <QComboBox>

// 记账条目结构体
struct AccountItem {
    QString book;       //账本
    QString date;      // 日期
    QString type;      // 类型：收入/支出
    QString category;  // 分类：餐饮、购物等
    double amount;     // 金额
    QString remark;    // 备注
};

namespace Ui {
class Record;
}

class Record : public QWidget
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent *event) override;  // 窗口显示时自动调用
public:
    explicit Record(QWidget *parent = nullptr);
    ~Record();

    void loadDataFromFile();   // 从文件加载数据（改为public，供外部调用）
    void updateCurrentBookDisplay(); // 更新当前账本显示

private slots:
    void on_sortTime_clicked();      // 按时间排序
    void on_sortCategory_clicked();  // 按分类排序
    void on_currentBookBtn_clicked(); // 点击当前账本切换
    void on_deleteRecord_clicked(int index); // 登记删除功能
    void on_editRecord_clicked(int index);   // 登记修改功能
    // 在 record.h 的 private 区域添加以下函数声明（如果还没有的话）

private:
    void saveSelectedBook();        // 保存选中的账本

    void displayRecords();     // 显示记录
    void updateTopbarStyle(QPushButton* activeBtn);  // 更新顶部按钮样式
    void loadSelectedBook();   // 加载选中的账本
    void loadBookNames();      // 加载账本名称列表（新增声明）
    void saveDataToFile();                   // 登记物理文件同步保存功能

    Ui::Record *ui;
    QList<AccountItem> m_allRecords;  // 所有记录
    int m_currentSortType = 0;        // 0:按时间 1:按分类

    // 当前账本相关
    QPushButton *m_currentBookBtn;     // 当前账本按钮
    QString m_currentBookName;         // 当前账本名称
    int m_selectedRow;                  // 选中的账本索引
    QStringList m_bookNames;            // 账本名称列表
};

#endif // RECORD_H