#ifndef DATACHARTWIDGET_H
#define DATACHARTWIDGET_H

#include <QWidget>
#include <QMap>

// 🌟 Qt 6 纯净原生前向声明：完全不需要任何 namespace 包裹！
class QTableWidget;
class QTabWidget;
class QChartView; // 像普通控件一样直接声明

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

    void refreshtable();
    void refreshbarchart();
    void refreshpiechart();

private:
    QTableWidget *m_tablewidget;
    QChartView   *m_barchartview; // 🌟 恢复最清爽的声明
    QChartView   *m_piechartview;
    QTabWidget   *m_tabwidget;

    QMap<QString, monthdata> m_monthlydata;
    QMap<QString, double>    m_categoryexpense;
    QString m_bookname;
};

#endif // DATACHARTWIDGET_H