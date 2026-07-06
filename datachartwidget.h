#ifndef DATACHARTWIDGET_H
#define DATACHARTWIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QMap>
#include <QPair>

// 兼容 Qt5 和 Qt6 的命名空间处理
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QT_CHARTS_USE_NAMESPACE
#endif

    class datachartwidget : public QWidget
{
    Q_OBJECT
public:
    explicit datachartwidget(QWidget *parent = nullptr);
    ~datachartwidget();

    void loaddata();                      // 供主窗口调用的核心数据加载与刷新函数
    void setbookname(const QString &name); // 满足你主页代码中的调用

private:
    QString getCurrentBook();             // 获取当前 JSON 选中的账本名

    // 显式声明图表视图容器
    QChartView *m_incomePieView;
    QChartView *m_outcomePieView;
    QChartView *m_trendLineView;

    QString m_bookName;
};

#endif // DATACHARTWIDGET_H