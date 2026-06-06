#include "budget.h"
#include "ui_budget.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTextStream>

Budget::Budget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Budget)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_StyledBackground, true);

    ui->dateEdit->setDisplayFormat("yyyy-MM");
    ui->dateEdit->setCalendarPopup(false);
    ui->dateEdit->setDate(QDate::currentDate());
    connect(ui->dateEdit, &QDateEdit::dateChanged, this, &Budget::loadData);

    ui->progressBar->setTextVisible(false);
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    ui->progressBar->setStyleSheet(
        "QProgressBar {"
        "   border: none;"
        "   background-color: #EAEAEA;"
        "   border-radius: 5px;"
        "   height: 8px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: #9E2A2B;"
        "   border-radius: 5px;"
        "}"
        );
    ui->labelHint->setStyleSheet(
        "color: #9E2A2B;"          /* 配合你进度条和保存按钮的国风深红 */
        "font-weight: bold;"       /* 加粗提示 */
        "font-size: 13px;"         /* 稍微大一点点的字号 */
        );

    loadData();
}

Budget::~Budget()
{
    delete ui;
}

// 获取当前选中的账本
QString Budget::getCurrentBook()
{
    QFile f("selected_book.json");
    if (f.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        return doc.object()["selectedBookName"].toString("我的账本");
    }
    return "我的账本";
}

// 读取当月已支出
double Budget::getMonthUsed(QString yearMonth)
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

void Budget::saveMonthBudget(QString yearMonth, double budget)
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
double Budget::loadMonthBudget(QString yearMonth)
{
    QString book = getCurrentBook();
    QFile f("budget/" + yearMonth + "_" + book + ".json");

    if (!f.open(QIODevice::ReadOnly)) return 0;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    return doc.object()["budget"].toDouble();
}

// 加载显示：预算、已使用、剩余
void Budget::loadData()
{
    QString ym = ui->dateEdit->date().toString("yyyy-MM");
    QString book = getCurrentBook();

    // 1. 精准判断文件是否存在
    bool hasBudgetFile = QFile::exists("budget/" + ym + "_" + book + ".json");

    double budget = loadMonthBudget(ym);
    double used = getMonthUsed(ym);
    double left = budget - used;

    ui->labelBudget->setText("预算：" + QString::number(budget, 'f', 2));
    ui->labelUsed->setText("已使用：" + QString::number(used, 'f', 2));
    ui->labelLeft->setText("剩余：" + QString::number(left, 'f', 2));

    // 无论有没有预算，标签都保持可见
    ui->labelHint->setVisible(true);

    // ==================== 核心修改：分情况显示文字和颜色 ====================
    if (!hasBudgetFile) {
        ui->progressBar->setValue(0);

        QString hintText = QString("提示：该账本（%1）当月（%2）暂无预算，请先设置预算").arg(book, ym);
        ui->labelHint->setText(hintText);
        ui->labelHint->setStyleSheet("color: #9E2A2B; font-weight: bold; font-size: 13px;");
    }
    else {
        QString infoText = QString("当前账本：%1  |  %2").arg(book, ym);
        ui->labelHint->setText(infoText);
        ui->labelHint->setStyleSheet("color: #555555; font-weight: normal; font-size: 13px;");

        // 正常计算进度条
        if (budget <= 0) {
            ui->progressBar->setValue(used > 0 ? 100 : 0);
        } else {
            double percentDouble = (used / budget) * 100;
            int percent = static_cast<int>(percentDouble);

            if (percent > 100) percent = 100;
            if (percent < 0) percent = 0;

            ui->progressBar->setValue(percent);
        }
    }
}

// 保存按钮
void Budget::on_btnSave_clicked()
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

void Budget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadData();
}