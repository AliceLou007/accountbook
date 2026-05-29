#include "addone.h"
#include "ui_addone.h"
#include <fstream>   // 引入标准文件流，用于写 data.txt
#include <QMessageBox>
#include <QDebug>

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

    // 1. 设置弹窗标题和固定大小（防止弹窗变形）
    this->setWindowTitle("添加收支记录");
    this->setFixedSize(400, 350); // 你可以根据界面控件调整这个大小

    // 2. 自动化加载：在程序启动时，自动往下拉框塞入“支出”和“收入”
    ui->comboBoxType->clear();
    ui->comboBoxType->addItem("支出");
    ui->comboBoxType->addItem("收入");
}

AddOne::~AddOne()
{
    delete ui;
}

// 当点击 OK 按钮时触发
void AddOne::on_buttonBox_accepted()
{
    AccountRecord rec;

    // 1. 抓取界面数据（保持不变）
    rec.date = ui->dateEdit->date().toString("yyyy-MM-dd").toStdString();
    rec.type = ui->comboBoxType->currentText().toLocal8Bit().constData();
    rec.category = ui->lineEditCategory->text().toLocal8Bit().constData();
    rec.amount = ui->lineEditAmount->text().toDouble();
    rec.comment = ui->lineEditComment->text().toLocal8Bit().constData();

    // 防御性拦截
    if (rec.category.empty() || rec.amount <= 0) {
        QMessageBox::warning(this, "提示", "请填写完整的分类和正确的金额！");
        return;
    }

    // 2. 写入文件（保持不变）
    std::ofstream outFile("data.txt", std::ios::app);
    if (outFile.is_open()) {
        outFile << rec.date << "\t"
                << rec.type << "\t"
                << rec.category << "\t"
                << rec.amount << "\t"
                << rec.comment << "\n";
        outFile.close();
    } else {
        QMessageBox::critical(this, "错误", "无法打开或创建账本文件！");
        return;
    }

    // 3. 完美的闭环：关闭弹窗，通知主界面变亮并刷新
    this->accept();
}

// 当用户点击了 Button Box 里的 “Cancel” 或者 “取消” 按钮时
void AddOne::on_buttonBox_rejected()
{
    // 直接关闭弹窗，撤走黑幕布，不保存任何东西
    this->reject();
}