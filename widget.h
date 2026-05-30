#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    // 刷新主页数据的核心功能函数，让外部（比如弹窗关闭后）也能调用


private slots:
    void on_btnHome_clicked();
    void on_btnHistory_clicked();

private:
    Ui::Widget *ui;

    // 1. 定义一个结构体，用来存放“某一个月”的统计账目
    struct MonthStat {
        double income = 0.0;   // 当月总收入
        double outcome = 0.0;  // 当月总支出
        double balance = 0.0;  // 当月结余 (income - outcome)
        int count = 0;         // 当月记账次数
    };

    // 2. 建立一个多月份大账本
    // 键（Key）格式为 "YYYY-MM"（例如 "2026-05"），值（Value）是对应的 MonthStat 结构体
    QMap<QString, MonthStat> allMonthsData;

    // 3. 当前主页正在查看的“年-月”（比如默认是当前月份）
    QString currentViewingMonth;

    // 4. 核心功能函数
    void loadAndCalculateAllData(); // 读取整个文件并分类统计到 Map 中
    void updateHomeUi(const QString &yearMonth); // 专门负责把某个月的数据刷到 UI 上
};
#endif // WIDGET_H