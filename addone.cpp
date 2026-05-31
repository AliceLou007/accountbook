#include "addone.h"
#include "ui_addone.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

AddOne::AddOne(QWidget *parent) :
    QDialog(parent), // 父类初始化
    ui(new Ui::AddOne)
{
    ui->setupUi(this);
    // 用 C++ 代码直接给整个弹窗设置 QSS 样式
    this->setStyleSheet(
        "QDialog {"
        "   background-color: #ffffff;" // 纯白背景
        "   border-radius: 12px;"       // 圆角
        "}"
        "QLineEdit {"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 6px;"
        "   padding: 6px;"
        "   background-color: #fcfcfc;"
        "}"
        "QLineEdit:focus {"
        "   border: 1.5px solid #8c1515;" // 聚焦时变成北大红
        "}"
        );

    // 1. 设置固定大小（防止弹窗变形）
    this->setFixedSize(400, 350);

    // 这一行会让弹窗内所有控件的文字都变成 16px，并使用微软雅黑字体
    this->setStyleSheet("font-size: 16px; font-family: 'Microsoft YaHei';");

    // 2. 自动化加载：在程序启动时，自动往下拉框塞入“支出”和“收入”
    ui->comboType->clear();
    ui->comboType->addItem("支出");
    ui->comboType->addItem("收入");
    //先让新的分类下拉框初始化（默认塞入支出分类）
    ui->comboCategory->clear();
    ui->comboCategory->addItems(QStringList() << "餐饮" << "购物" << "日用" << "交通" << "娱乐" << "医疗" << "其他");

    ui->lineEditComment->setEditable(true);
    ui->lineEditComment->clear();
    ui->lineEditComment->addItems(getTopThreeRemarks());
    ui->lineEditComment->setCurrentText("");

    //直接绑架你原本就有的那个大类选择框（假设叫 comboType）
    connect(ui->comboType, &QComboBox::currentTextChanged, this, [=](const QString &type){
        ui->comboCategory->clear();
        if (type == "支出") {
            ui->comboCategory->addItems(QStringList() << "餐饮" << "购物" << "日用" << "交通" << "娱乐" << "医疗" << "其他");
        }
        else if (type == "收入") {
            ui->comboCategory->addItems(QStringList() << "工资" << "生活费" << "奖学金" << "兼职" << "理财" << "其他");
        }
    });
}

AddOne::~AddOne()
{
    delete ui;
}

// 当点击 OK 按钮时触发
void AddOne::on_buttonBox_accepted()
{
    // 1. 检查输入合法性（先做检查，如果不合法直接拦截，省去后面所有无用功）
    QString category = ui->comboCategory->currentText();
    double amount = ui->lineEditAmount->text().toDouble();

    if (category.isEmpty() || amount <= 0) {
        QMessageBox::warning(this, "提示", "请填写完整的分类和正确的金额！");
        return; // 被拦截，不关闭弹窗，不写入文件
    }

    // 2. 使用 Qt 的组件进行写入，彻底消灭中文乱码和路径错位问题
    // 使用 applicationDirPath() 确保主界面和弹窗读写的是【同一个绝对路径】下的 data.txt
    QString filePath = QCoreApplication::applicationDirPath() + "/data.txt";
    QFile file(filePath);

    // 以追加模式(Append)和文本模式(Text)打开
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        // 用逗号 `,` 分隔。备注如果为空，默认写“无”
        QString comment = ui->lineEditComment->currentText();
        if (comment.isEmpty()) comment = "无";

        // 啪！整整齐齐写进文件，Qt 会在后台自动处理好所有中文编码，绝对不会乱码
        out << ui->dateEdit->date().toString("yyyy-MM-dd") << ","
            << ui->comboType->currentText() << ","
            << category << ","
            << amount << ","
            << comment << "\n";

        file.close();
    } else {
        QMessageBox::critical(this, "错误", "无法打开或创建账本文件！");
        return;
    }

    // 3. 完美的闭环：关闭弹窗，通知主界面变亮并刷新
    this->accept();
}


QStringList AddOne::getTopThreeRemarks() {
    QFile file("data.txt"); // 确认你的 data.txt 路径正确
    QMap<QString, int> remarkCount;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "打不开 data.txt 啦！";
        return QStringList();
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() >= 4) {
            QString remark = parts[2].trimmed();
            if (!remark.isEmpty()) {
                remarkCount[remark]++;
            }
        }
    }
    file.close();

    // 接下来：把 Map 里的数据按照出现次数进行降序排序
    // 我们可以把它们倒腾到一个 QList 里方便用 std::sort
    QList<QPair<QString, int>> sortedList;
    for (auto it = remarkCount.begin(); it != remarkCount.end(); ++it) {
        sortedList.append(qMakePair(it.key(), it.value()));
    }

    std::sort(sortedList.begin(), sortedList.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second > b.second;
    });

    QStringList topThree;
    for (int i = 0; i < qMin(3, sortedList.size()); ++i) {
        topThree.append(sortedList[i].first);
    }

    return topThree;
}

// 当用户点击了 Button Box 里的 “Cancel” 或者 “取消” 按钮时
void AddOne::on_buttonBox_rejected()
{
    // 直接关闭弹窗，撤走黑幕布，不保存任何东西
    this->reject();
}