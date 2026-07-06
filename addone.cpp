#include "addone.h"
#include "ui_addone.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QFileInfo>
#include <QFileDialog>
#include <QPixmap>
#include <QHBoxLayout>
#include <QPushButton>
#include "userdata.h"

AddOne::AddOne(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddOne),
    m_imageLabel(nullptr)
{
    ui->setupUi(this);

    // 设置日期默认为今天
    ui->dateEdit->setDate(QDate::currentDate());

    this->setFixedSize(400, 450);
    this->setStyleSheet("font-size: 16px; font-family: 'Microsoft YaHei';");

    // 加载账本列表
    loadBookNames();

    ui->comboType->clear();
    ui->comboType->addItem("支出");
    ui->comboType->addItem("收入");

    // 加载分类标签
    loadCategoriesFromTags();

    ui->lineEditComment->setEditable(true);
    ui->lineEditComment->clear();
    ui->lineEditComment->addItems(getTopThreeRemarks());
    ui->lineEditComment->setCurrentText("");

    // 连接信号
    connect(ui->comboType, &QComboBox::currentTextChanged, this, &AddOne::onTypeChanged);

    // ========== 连接图片按钮 ==========
    connect(ui->picture, &QPushButton::clicked, this, &AddOne::onSelectImage);

    m_currentImagePath = "";
}
AddOne::~AddOne()
{
    delete ui;
}

// 选择图片
void AddOne::onSelectImage()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "选择图片",
                                                    QDir::homePath(),
                                                    "图片文件 (*.png *.jpg *.jpeg *.bmp)");

    if (!filePath.isEmpty()) {
        m_currentImagePath = filePath;
        if (m_imageLabel) {
            QString fileName = QFileInfo(filePath).fileName();
            m_imageLabel->setText(fileName);
            m_imageLabel->setStyleSheet("color: #8c1515; font-size: 12px;");
        }
    }
}

// 复制图片到账本目录
QString AddOne::copyImageToBook(const QString &imagePath, const QString &bookName)
{
    if (imagePath.isEmpty()) return "";

    // 创建图片保存目录
    QString imageDir = UserData::imageDir(bookName);
    QDir dir;
    if (!dir.exists(imageDir)) {
        dir.mkpath(imageDir);
    }

    // 生成唯一文件名
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fileName = timestamp + "_" + QFileInfo(imagePath).fileName();
    QString destPath = imageDir + "/" + fileName;

    // 复制图片
    if (QFile::copy(imagePath, destPath)) {
        return destPath;
    }

    return "";
}

// 加载分类标签
void AddOne::loadCategoriesFromTags()
{
    ui->comboCategory->clear();

    QStringList expenseTags;
    QStringList incomeTags;

    QString tagFilePath = UserData::tagsFile();
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

    if (expenseTags.isEmpty()) {
        expenseTags = {"餐饮", "购物", "日用", "交通", "娱乐", "医疗", "其他"};
    }
    if (incomeTags.isEmpty()) {
        incomeTags = {"工资", "生活费", "奖学金", "兼职", "理财", "其他"};
    }

    m_expenseTags = expenseTags;
    m_incomeTags = incomeTags;

    QString currentType = ui->comboType->currentText();
    if (currentType == "支出") {
        ui->comboCategory->addItems(m_expenseTags);
    } else {
        ui->comboCategory->addItems(m_incomeTags);
    }
}

// 类型改变
void AddOne::onTypeChanged(const QString &type)
{
    ui->comboCategory->clear();
    if (type == "支出") {
        ui->comboCategory->addItems(m_expenseTags);
    } else if (type == "收入") {
        ui->comboCategory->addItems(m_incomeTags);
    }
}

// 加载账本名称
void AddOne::loadBookNames()
{
    ui->comboBook->clear();

    QString filePath = UserData::booksFile();
    QFile file(filePath);

    if (!file.exists()) {
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
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        QString bookName = obj["name"].toString();
        if (!bookName.isEmpty()) {
            ui->comboBook->addItem(bookName);
        }
    }

    loadSelectedBook();
}

// 加载选中的账本
void AddOne::loadSelectedBook()
{
    QString filePath = UserData::selectedBookFile();
    QFile file(filePath);

    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString savedBookName = obj["selectedBookName"].toString();

    if (!savedBookName.isEmpty()) {
        int index = ui->comboBook->findText(savedBookName);
        if (index >= 0) {
            ui->comboBook->setCurrentIndex(index);
        }
    }
}

// 提交记录
void AddOne::on_buttonBox_accepted()
{
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

    // 处理图片
    QString savedImagePath = copyImageToBook(m_currentImagePath, selectedBook);

    // 保存记录
    QString fileName = QString("%1_data.txt").arg(selectedBook);
    QString filePath = UserData::recordFile(selectedBook);
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);

        QString comment = ui->lineEditComment->currentText();
        if (comment.isEmpty()) comment = "无";

        out << selectedBook << ","
            << ui->dateEdit->date().toString("yyyy-MM-dd") << ","
            << ui->comboType->currentText() << ","
            << category << ","
            << amount << ","
            << comment << ","
            << savedImagePath << "\n";

        file.close();
        updateBookRecordCount(selectedBook);

        this->accept();
    } else {
        QMessageBox::critical(this, "错误", QString("无法打开账本文件：%1").arg(fileName));
    }
}

// 更新记录条数
// 更新账本的记录条数
void AddOne::updateBookRecordCount(const QString &bookName)
{
    QString booksFilePath = UserData::booksFile();
    QFile booksFile(booksFilePath);

    if (!booksFile.open(QIODevice::ReadOnly)) return;

    QByteArray data = booksFile.readAll();
    booksFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;

    QJsonArray array = doc.array();

    // 统计该账本的记录条数
    QString dataFilePath = UserData::recordFile(bookName);
    QFile dataFile(dataFilePath);

    int recordCount = 0;
    if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&dataFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) recordCount++;
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

    if (booksFile.open(QIODevice::WriteOnly)) {
        QJsonDocument newDoc(array);
        booksFile.write(newDoc.toJson());
        booksFile.close();
    }
}

// 获取常用备注
QStringList AddOne::getTopThreeRemarks()
{
    QString selectedBook = ui->comboBook->currentText();
    if (selectedBook.isEmpty()) return QStringList();

    QFile file(UserData::recordFile(selectedBook));
    QMap<QString, int> remarkCount;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QStringList();

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() >= 6) {
            QString remark = parts[5].trimmed();
            if (remark.endsWith("\n")) remark.chop(1);
            if (!remark.isEmpty() && remark != "无" && remark != "\n") {
                remarkCount[remark]++;
            }
        }
    }
    file.close();

    QList<QPair<QString, int>> sortedList;
    for (auto it = remarkCount.begin(); it != remarkCount.end(); ++it) {
        sortedList.append(qMakePair(it.key(), it.value()));
    }

    std::sort(sortedList.begin(), sortedList.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
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
