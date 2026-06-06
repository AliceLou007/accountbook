#ifndef TABLESTAT_H
#define TABLESTAT_H

#include <QDialog>
#include <QTableWidget>
#include <QList>
#include <QStringList>

class tablereport : public QDialog
{
    Q_OBJECT
public:
    explicit tablereport(QWidget *parent = nullptr);
    // 传字符串二维表，不再依赖MonthStat
    void set_data(const QList<QStringList>& data);
private:
    QTableWidget *table;
};

#endif