#include "buget.h"
#include "ui_buget.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTextStream>

buget::buget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::buget)
{
    ui->setupUi(this);
    loadData();
}

buget::~buget()
{
    delete ui;
}

// 获取当前选中的账本
QString buget::getCurrentBook()
{
    QFile f("selected_book.json");
    if (f.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        return doc.object()["selectedBookName"].toString("我的账本");
    }
    return "我的账本";
}

// 读取当月已支出（从你原来的账本文件读取，不用新文件）
double buget::getMonthUsed(QString yearMonth)
{
    QString book = getCurrentBook();
    QFile f(book + "_data.txt");

    if (!f.open(QIODevice::ReadOnly)) return 0;

    QTextStream in(&f);
    double total = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList list = line.split(",");
        if (list.size() >= 6) {
            QString date = list[1];
            QString ym = date.left(7).replace("/", "-");
            QString type = list[2];
            double money = list[4].toDouble();

            if (ym == yearMonth && type == "支出") {
                total += money;
            }
        }
    }
    f.close();
    return total;
}

// 保存预算：按月保存到 budget/2025-09_我的账本.json
void buget::saveMonthBudget(QString yearMonth, double budget)
{
    QDir().mkpath("budget");

    QString book = getCurrentBook();
    QFile f("budget/" + yearMonth + "_" + book + ".json");

    if (f.open(QIODevice::WriteOnly)) {
        QJsonObject obj;
        obj["budget"] = budget;
        f.write(QJsonDocument(obj).toJson());
        f.close();
    }
}

// 读取当月预算
double buget::loadMonthBudget(QString yearMonth)
{
    QString book = getCurrentBook();
    QFile f("budget/" + yearMonth + "_" + book + ".json");

    if (!f.open(QIODevice::ReadOnly)) return 0;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    return doc.object()["budget"].toDouble();
}

// 加载显示：预算、已使用、剩余
void buget::loadData()
{
    QString ym = ui->dateEdit->date().toString("yyyy-MM");

    double budget = loadMonthBudget(ym);
    double used = getMonthUsed(ym);
    double left = budget - used;

    ui->labelBudget->setText("预算：" + QString::number(budget, 'f', 2));
    ui->labelUsed->setText("已使用：" + QString::number(used, 'f', 2));
    ui->labelLeft->setText("剩余：" + QString::number(left, 'f', 2));
}

// 保存按钮
void buget::on_btnSave_clicked()
{
    QString ym = ui->dateEdit->date().toString("yyyy-MM");
    bool ok;
    double budget = ui->lineEditBudget->text().toDouble(&ok);

    if (!ok || budget <= 0) {
        QMessageBox::warning(this, "错误", "请输入有效预算！");
        return;
    }

    saveMonthBudget(ym, budget);
    loadData();
    QMessageBox::information(this, "成功", "预算已按月保存！");
}