#ifndef BUGET_H
#define BUGET_H

#include <QWidget>

namespace Ui {
class buget;
}

class buget : public QWidget
{
    Q_OBJECT

public:
    explicit buget(QWidget *parent = nullptr);
    ~buget();

private slots:
    void on_btnSave_clicked();
    void loadData();

private:
    Ui::buget *ui;

    QString getCurrentBook();
    double getMonthUsed(QString yearMonth);
    void saveMonthBudget(QString yearMonth, double budget);
    double loadMonthBudget(QString yearMonth);
};

#endif // BUGET_H