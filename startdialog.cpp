#include "startdialog.h"
#include "datachartwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>

startdialog::startdialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("账本工具");
    resize(350, 250);   // 适当增大一点，避免按钮过于局促

    m_btn = new QPushButton("显示统计图表", this);
    connect(m_btn, &QPushButton::clicked, this, &startdialog::on_showchart_clicked);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_btn, 0, Qt::AlignCenter);
}

void startdialog::on_showchart_clicked()
{
    QDialog *chartdialog = new QDialog(this);
    chartdialog->setWindowTitle("账本统计分析");
    chartdialog->resize(900, 650);   // 修改为更合适的尺寸

    datachartwidget *chartwidget = new datachartwidget(chartdialog);
    chartwidget->loaddata();

    QVBoxLayout *layout = new QVBoxLayout(chartdialog);
    layout->addWidget(chartwidget);
    chartdialog->setLayout(layout);

    chartdialog->exec();
}