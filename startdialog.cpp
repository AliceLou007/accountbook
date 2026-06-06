#include "startdialog.h"
#include "datachartwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>

startdialog::startdialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("账本工具");
    resize(300, 200);

    m_btn = new QPushButton("显示统计图表", this);
    connect(m_btn, &QPushButton::clicked, this, &startdialog::on_showchart_clicked);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_btn, 0, Qt::AlignCenter);
}

void startdialog::on_showchart_clicked()
{
    // 创建包含图表的大窗口
    QDialog *chartdialog = new QDialog(this);
    chartdialog->setWindowTitle("账本统计分析");
    chartdialog->resize(1000, 700);

    datachartwidget *chartwidget = new datachartwidget(chartdialog);
    chartwidget->loaddata();   // 自动读取 selected_book.json

    QVBoxLayout *layout = new QVBoxLayout(chartdialog);
    layout->addWidget(chartwidget);
    chartdialog->setLayout(layout);

    chartdialog->exec();   // 模态显示，等待关闭
}