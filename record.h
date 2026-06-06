#ifndef RECORD_H
#define RECORD_H

#include <QWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStringList>
#include <QShowEvent>

// 记账条目结构体
struct AccountItem {
    QString book;       // 账本
    QString date;       // 日期
    QString type;       // 类型：收入/支出
    QString category;   // 分类：餐饮、购物等
    double amount;      // 金额
    QString remark;     // 备注
};

namespace Ui {
class Record;
}

class Record : public QWidget
{
    Q_OBJECT

public:
    explicit Record(QWidget *parent = nullptr);
    ~Record();

    void loadDataFromFile();           // 从文件加载数据（public，供外部调用）
    void updateCurrentBookDisplay();   // 更新当前账本显示

signals:
    void dataChanged();                // 数据改变信号

protected:
    void showEvent(QShowEvent *event) override;  // 窗口显示时自动调用

private slots:
    void on_currentBookBtn_clicked();  // 点击当前账本切换
    void on_sortTime_clicked();        // 按时间排序
    void on_editRecord_clicked(int index);   // 登记修改功能
    void on_deleteRecord_clicked(int index); // 登记删除功能

private:
    void loadBookNames();              // 加载账本名称列表
    void loadSelectedBook();           // 加载选中的账本
    void saveSelectedBook();           // 保存选中的账本
    void saveDataToFile();             // 登记物理文件同步保存功能
    void displayRecords();             // 显示记录
    void filterAndDisplayRecords(const QString &selectedCategory); // 按分类筛选显示
    void updateTopbarStyle(QPushButton* activeBtn);  // 更新顶部按钮样式
    void loadTagsFromFile();           // 加载标签配置
    void checkAndFixCategories();      // 检查并修复已删除的标签
    void updateCategoryMenu();         // 更新分类菜单

    Ui::Record *ui;
    QList<AccountItem> m_allRecords;   // 所有记录
    QStringList m_bookNames;           // 账本名称列表
    QString m_currentBookName;         // 当前账本名称
    int m_selectedRow;                 // 选中的账本索引
    int m_currentSortType;             // 0:按时间 1:按分类

    QPushButton *m_currentBookBtn;     // 当前账本按钮
};

#endif // RECORD_H