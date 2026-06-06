#ifndef DATACHARTWIDGET_H
#define DATACHARTWIDGET_H

#include <QWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
class QChartView;
QT_END_NAMESPACE

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

    void refreshlinechart();   // 折线图
    void refreshpiechart();    // 饼状图

private:
    QChartView *m_linechartview;   // 折线图视图
    QChartView *m_piechartview;    // 饼状图视图

    QMap<QString, monthdata> m_monthlydata;
    QMap<QString, double>    m_categoryexpense;
    QString m_bookname;
};

#endif // DATACHARTWIDGET_H