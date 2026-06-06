#include "tablestat.h"
#include <QVBoxLayout>

tablereport::tablereport(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("月度统计");
    resize(700, 400);
    table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"月份","收入","支出","结余","条数"});
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(table);
    setLayout(layout);
}

void tablereport::set_data(const QList<QStringList>& data)
{
    table->setRowCount(data.size());
    for(int i=0;i<data.size();i++)
    {
        for(int j=0;j<5;j++)
        {
            table->setItem(i,j,new QTableWidgetItem(data[i][j]));
        }
    }
}