#include "widget.h"
#include "ui_widget.h"
#include "addone.h"
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QCoreApplication>
#include<QPixmap>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);


    this->setFixedSize(950, 850);
    ui->stackedWidget->setCurrentIndex(0);

    currentViewingMonth = QDate::currentDate().toString("yyyy-MM"); // 拿到系统当月
    updateHomeUi(currentViewingMonth); // 展现当月数据
    updateSidebarStyle(ui->btnHome);//默认主页变红

    //  大红按钮点击事件(添加一笔收支）
    connect(ui->btnOpenAdd, &QPushButton::clicked, this, [=](){
        QWidget *mask = new QWidget(this);
        mask->setGeometry(this->rect());
        mask->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
        mask->show();

        AddOne dialog(this);
        dialog.setFixedSize(450, 400);
        int x = this->x() + (this->width() - dialog.width()) / 2;
        int y = this->y() + (this->height() - dialog.height()) / 2;
        dialog.move(x, y);

        if (dialog.exec() == QDialog::Accepted) {
            // 当弹窗成功写入 data.txt 并关闭后，主页面立刻重新读文件、刷新数字！
            updateHomeUi(currentViewingMonth);
        }

        mask->close();
        delete mask;
    });
}

Widget::~Widget()
{
    delete ui;
}

//  编写完整的算账刷新函数
void Widget::loadAndCalculateAllData()
{
    allMonthsData.clear(); // 清空旧统计

    QString filePath = QCoreApplication::applicationDirPath() + "/data.txt";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return; // 文件不存在直接返回
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList tokens = line.split(",");

        // 确保一行切出了 5 个字段
        if (tokens.size() >= 5) {
            QString qDate = tokens[0];
            QString qType = tokens[1];
            QString qCategory = tokens[2];
            double amount = tokens[3].toDouble();
            QString qComment = tokens[4];

            // 提取年月 Key (如 "2026-05")
            QString yearMonthKey = qDate.left(7).replace("/", "-");


            if (qType == "收入") {
                allMonthsData[yearMonthKey].income += amount;
            } else if (qType == "支出") {
                allMonthsData[yearMonthKey].outcome += amount;
            }
            allMonthsData[yearMonthKey].count++;
        }
    }
    file.close();

    // 重新计算结余
    for (auto it = allMonthsData.begin(); it != allMonthsData.end(); ++it) {
        it.value().balance = it.value().income - it.value().outcome;
    }
}
// 2. 核心：给它一个 "YYYY-MM"，它就去 Map 里抓数据刷到界面上
void Widget::updateHomeUi(const QString &yearMonth)
{
    // 1. 调用读文件函数
    loadAndCalculateAllData();

    // 2. 如果 Map 里根本没有这个月的数据（说明这个月没记过账）
    if (!allMonthsData.contains(yearMonth)) {
        ui->lblIncomeAmount->setText("￥ 0.00");
        ui->lblOutcomeAmount->setText("￥ 0.00");
        ui->lblBalanceAmount->setText("￥ 0.00");
        ui->lblCount->setText("本月记账: 0 笔");
        return;
    }

    // 3. 获取该月份的结构体数据
    const MonthStat &stat = allMonthsData[yearMonth];

    // 4. 啪！刷新进界面
    ui->lblIncomeAmount->setText("￥ " + QString::number(stat.income, 'f', 2));
    ui->lblOutcomeAmount->setText("￥ " + QString::number(stat.outcome, 'f', 2));
    ui->lblBalanceAmount->setText("￥ " + QString::number(stat.balance, 'f', 2));
    ui->lblCount->setText(QString::number(stat.count) + " 笔");
}

// 侧边栏翻页 + 变色联动
void Widget::on_btnHome_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
    updateSidebarStyle(ui->btnHome);
}
void Widget::on_btnHistory_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
    updateSidebarStyle(ui->btnHistory);
}
void Widget::on_btnCount_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    updateSidebarStyle(ui->btnCount);
}
void Widget::on_btnBudget_clicked() {
    ui->stackedWidget->setCurrentIndex(3);
    updateSidebarStyle(ui->btnBudget);
}
void Widget::on_btnManagement_clicked() {
    ui->stackedWidget->setCurrentIndex(4);
    updateSidebarStyle(ui->btnManagement);
}
void Widget::updateSidebarStyle(QPushButton* activeBtn)
{

    QList<QPushButton*> sidebarBtns = {
        ui->btnHome,
        ui->btnHistory,
        ui->btnCount,
        ui->btnBudget,
        ui->btnManagement
    };

    QString activeStyle = "QPushButton { "
                          "background-color: #8c1515;"
                          "color: white; "
                          "border: none; "
                          "font-weight: bold;"
                          "}";

    QString inactiveStyle = "QPushButton { "
                            "background-color: #FFFFFF; "
                            "color: #333333; "
                            "font-weight: bold"
                            "border: none; "
                            "} "
                            "QPushButton:hover { "
                            "background-color: #FFF0F2; "
                            "}";

    // 遍历所有按钮，动态发牌
    for (QPushButton* btn : sidebarBtns) {
        if (btn == activeBtn) {
            btn->setStyleSheet(activeStyle);   // 命中的变红
        } else {
            btn->setStyleSheet(inactiveStyle); // 没命中的变白
        }
    }
}