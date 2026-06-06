#ifndef DATACHARTWIDGET_H
#define DATACHARTWIDGET_H

#include <QWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
// 不再需要 QTableWidget，可以移除前向声明
// class QTableWidget;
QT_END_NAMESPACE

QT_CHARTS_BEGIN_NAMESPACE
    class QChartView;
QT_CHARTS_END_NAMESPACE

    struct monthdata {
    double income = 0.0;
    double outcome = 0.0;
    double balance = 0.0;
    int count = 0;
};

class datachartwidget : public QWidget
{
    Q_OBJECT
public:
    explicit datachartwidget(QWidget *parent = nullptr);

    void setbookname(const QString &name);
    void loaddata();

protected:
    void setupui();
    void parsedatafile();
    QString getdatafilepath() const;

    void refreshbarchart();   // 柱状图
    void refreshpiechart();   // 饼状图

private:
    // QTableWidget *m_tablewidget;  // 移除表格指针
    QChartView   *m_barchartview;
    QChartView   *m_piechartview;

    QMap<QString, monthdata> m_monthlydata;
    QMap<QString, double>    m_categoryexpense;
    QString m_bookname;
};

#endif // DATACHARTWIDGET_H