#ifndef BUDGET_H
#define BUDGET_H

#include <QWidget>

namespace Ui {
class Budget;
}

class Budget : public QWidget
{
    Q_OBJECT

public:
    explicit Budget(QWidget *parent = nullptr);
    ~Budget();

private slots:
    void on_btnSave_clicked();
    void loadData();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::Budget *ui;

    QString getCurrentBook();
    double getMonthUsed(QString yearMonth);
    void saveMonthBudget(QString yearMonth, double budget);
    double loadMonthBudget(QString yearMonth);
};

#endif // BUDGET_H