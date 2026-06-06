#ifndef DATACHARTWIDGET_H
#define DATACHARTWIDGET_H

#include <QWidget>
#include <QMap>

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
    void refreshpiecharts();

private:
    class QChartView *m_incomeChartView;
    class QChartView *m_expenseChartView;

    QMap<QString, double> m_incomeByCategory;
    QMap<QString, double> m_expenseByCategory;
    QString m_bookname;
};

#endif // DATACHARTWIDGET_H