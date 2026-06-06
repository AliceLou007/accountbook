#include "addone.h"
#include "ui_addone.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

AddOne::AddOne(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddOne)
{
    ui->setupUi(this);

    // ========== 设置日期默认为今天 ==========
    ui->dateEdit->setDate(QDate::currentDate());

    // 设置样式
    this->setStyleSheet(
        "QDialog {"
        "   background-color: #ffffff;"
        "   border-radius: 12px;"
        "}"
        "QLineEdit {"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 6px;"
        "   padding: 6px;"
        "   background-color: #fcfcfc;"
        "}"
        "QLineEdit:focus {"
        "   border: 1.5px solid #8c1515;"
        "}"
        );

    this->setFixedSize(400, 350);
    this->setStyleSheet("font-size: 16px; font-family: 'Microsoft YaHei';");

    // 加载账本列表
    loadBookNames();

    ui->comboType->clear();
    ui->comboType->addItem("支出");
    ui->comboType->addItem("收入");

    // 从 tags.json 加载分类标签
    loadCategoriesFromTags();

    ui->lineEditComment->setEditable(true);
    ui->lineEditComment->clear();
    ui->lineEditComment->addItems(getTopThreeRemarks());
    ui->lineEditComment->setCurrentText("");

    // 连接类型改变信号
    connect(ui->comboType, &QComboBox::currentTextChanged, this, &AddOne::onTypeChanged);
}

AddOne::~AddOne()
{
    delete ui;
}

// 从 tags.json 加载分类
void AddOne::loadCategoriesFromTags()
{
    ui->comboCategory->clear();

    QStringList expenseTags;
    QStringList incomeTags;

    QString tagFilePath = QDir::currentPath() + "/tags.json";
    QFile file(tagFilePath);

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            QJsonObject obj = doc.object();

            if (obj.contains("expenseTags") && obj["expenseTags"].isArray()) {
                QJsonArray expenseArray = obj["expenseTags"].toArray();
                for (const QJsonValue &value : expenseArray) {
                    if (value.isString()) {
                        expenseTags << value.toString();
                    }
                }
            }

            if (obj.contains("incomeTags") && obj["incomeTags"].isArray()) {
                QJsonArray incomeArray = obj["incomeTags"].toArray();
                for (const QJsonValue &value : incomeArray) {
                    if (value.isString()) {
                        incomeTags << value.toString();
                    }
                }
            }
        }
        file.close();
    }

    // 如果读取失败，使用默认标签
    if (expenseTags.isEmpty()) {
        expenseTags = {"餐饮", "购物", "日用", "交通", "娱乐", "医疗", "其他"};
    }
    if (incomeTags.isEmpty()) {
        incomeTags = {"工资", "生活费", "奖学金", "兼职", "理财", "其他"};
    }

    // 保存到成员变量
    m_expenseTags = expenseTags;
    m_incomeTags = incomeTags;

    // 根据当前选择的类型显示对应的分类
    QString currentType = ui->comboType->currentText();
    if (currentType == "支出") {
        ui->comboCategory->addItems(m_expenseTags);
    } else {
        ui->comboCategory->addItems(m_incomeTags);
    }
}

// 类型改变时的处理
void AddOne::onTypeChanged(const QString &type)
{
    ui->comboCategory->clear();
    if (type == "支出") {
        ui->comboCategory->addItems(m_expenseTags);
    } else if (type == "收入") {
        ui->comboCategory->addItems(m_incomeTags);
    }
}

// 从 books.json 加载账本名称
void AddOne::loadBookNames()
{
    ui->comboBook->clear();

    QString filePath = QDir::currentPath() + "/books.json";
    QFile file(filePath);

    if (!file.exists()) {
        // 如果没有账本文件，添加默认账本
        ui->comboBook->addItem("我的账本");
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开 books.json";
        ui->comboBook->addItem("我的账本");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        ui->comboBook->addItem("我的账本");
        return;
    }

    QJsonArray array = doc.array();
    if (array.isEmpty()) {
        ui->comboBook->addItem("我的账本");
        return;
    }

    // 添加所有账本名称
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        QString bookName = obj["name"].toString();
        if (!bookName.isEmpty()) {
            ui->comboBook->addItem(bookName);
        }
    }

    // 尝试加载选中的账本
    loadSelectedBook();
}

// 加载选中的账本
void AddOne::loadSelectedBook()
{
    QString filePath = QDir::currentPath() + "/selected_book.json";
    QFile file(filePath);

    if (!file.exists()) {
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject obj = doc.object();
    QString savedBookName = obj["selectedBookName"].toString();

    if (!savedBookName.isEmpty()) {
        int index = ui->comboBook->findText(savedBookName);
        if (index >= 0) {
            ui->comboBook->setCurrentIndex(index);
        }
    }
}

// 当点击 OK 按钮时触发
void AddOne::on_buttonBox_accepted()
{
    // 检查输入合法性
    QString category = ui->comboCategory->currentText();
    double amount = ui->lineEditAmount->text().toDouble();
    QString selectedBook = ui->comboBook->currentText();

    if (selectedBook.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择一个账本！");
        return;
    }

    if (category.isEmpty() || amount <= 0) {
        QMessageBox::warning(this, "提示", "请填写完整的分类和正确的金额！");
        return;
    }

    // 使用账本名称作为文件名
    QString fileName = QString("%1_data.txt").arg(selectedBook);
    QString filePath = QDir::currentPath() + "/" + fileName;
    QFile file(filePath);

    // 以追加模式打开
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        QString comment = ui->lineEditComment->currentText();
        if (comment.isEmpty()) comment = "无";

        // 格式: 账本名称,日期,类型,分类,金额,备注
        out << selectedBook << ","
            << ui->dateEdit->date().toString("yyyy-MM-dd") << ","
            << ui->comboType->currentText() << ","
            << category << ","
            << amount << ","
            << comment << "\n";

        file.close();

        // 更新该账本的记录条数
        updateBookRecordCount(selectedBook);

    } else {
        QMessageBox::critical(this, "错误", QString("无法打开或创建账本文件：%1").arg(fileName));
        return;
    }

    // 关闭弹窗
    this->accept();
}

// 更新账本的记录条数
void AddOne::updateBookRecordCount(const QString &bookName)
{
    QString booksFilePath = QDir::currentPath() + "/books.json";
    QFile booksFile(booksFilePath);

    if (!booksFile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = booksFile.readAll();
    booksFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return;
    }

    QJsonArray array = doc.array();

    // 统计该账本的记录条数
    QString dataFileName = QString("%1_data.txt").arg(bookName);
    QString dataFilePath = QDir::currentPath() + "/" + dataFileName;
    QFile dataFile(dataFilePath);

    int recordCount = 0;
    if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&dataFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                recordCount++;
            }
        }
        dataFile.close();
    }

    // 更新 books.json 中的记录条数
    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array[i].toObject();
        if (obj["name"].toString() == bookName) {
            obj["recordCount"] = recordCount;
            array[i] = obj;
            break;
        }
    }

    // 保存更新后的数据
    if (booksFile.open(QIODevice::WriteOnly)) {
        QJsonDocument newDoc(array);
        booksFile.write(newDoc.toJson());
        booksFile.close();
    }
}

QStringList AddOne::getTopThreeRemarks() {
    // 获取当前选中的账本
    QString selectedBook = ui->comboBook->currentText();
    if (selectedBook.isEmpty()) {
        return QStringList();
    }

    QString fileName = QString("%1_data.txt").arg(selectedBook);
    QString filePath = QDir::currentPath() + "/" + fileName;
    QFile file(filePath);
    QMap<QString, int> remarkCount;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "打不开账本文件：" << filePath;
        return QStringList();
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() >= 6) {
            QString remark = parts[5].trimmed();

            // 修复：去除末尾的 \\n
            if (remark.endsWith("\\n")) {
                remark.chop(1);
            }

            if (!remark.isEmpty() && remark != "无" && remark != "\\n") {
                remarkCount[remark]++;
            }
        }
    }
    file.close();

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

void AddOne::on_buttonBox_rejected()
{
    this->reject();
}