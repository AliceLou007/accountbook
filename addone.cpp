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
    // 1. 检查输入合法性（先做检查，如果不合法直接拦截，省去后面所有无用功）
    QString category = ui->lineEditCategory->text().trimmed();
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
        QString comment = ui->lineEditComment->text().trimmed();
        if (comment.isEmpty()) comment = "无";

        // 啪！整整齐齐写进文件，Qt 会在后台自动处理好所有中文编码，绝对不会乱码
        out << ui->dateEdit->date().toString("yyyy-MM-dd") << ","
            << ui->comboBoxType->currentText() << ","
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

// 当用户点击了 Button Box 里的 “Cancel” 或者 “取消” 按钮时
void AddOne::on_buttonBox_rejected()
{
    // 直接关闭弹窗，撤走黑幕布，不保存任何东西
    this->reject();
}