#include "statstablewindow.h"
#include "monthdatareader.h"
#include <QVBoxLayout>
#include <QTableWidgetItem>

statstablewindow::statstablewindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("月度收支统计表");
    setMinimumSize(750, 450);

    table = new QTableWidget;
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({
        "月份", "总收入", "总支出", "结余", "账单数"
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(table);
    setLayout(layout);

    loadAndShow();
}

void statstablewindow::loadAndShow()
{
    monthdatareader reader;
    auto data = reader.loadData();

    table->setRowCount(0);
    int row = 0;

    for (auto it = data.begin(); it != data.end(); ++it) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(it.key()));
        table->setItem(row, 1, new QTableWidgetItem(QString::number(it.value().income, 'f', 2)));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(it.value().outcome, 'f', 2)));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(it.value().balance, 'f', 2)));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(it.value().count)));
        row++;
    }
}