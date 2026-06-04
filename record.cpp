#include "record.h"
#include "ui_record.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QShowEvent>

Record::Record(QWidget *parent) : QWidget(parent), ui(new Ui::Record) {
    ui->setupUi(this);

    // ========== 添加红底白字抬头 ==========
    QWidget *topWidget = new QWidget(this);
    topWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #8c1515;"
        "   border: none;"
        "}"
        );
    topWidget->setFixedHeight(70);
    topWidget->setFixedWidth(700);

    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(150, 10, 50, 0);
    topLayout->setSpacing(50);

    QLabel *currentBookTitle = new QLabel("当前账本：", topWidget);
    currentBookTitle->setFixedWidth(80);
    currentBookTitle->setStyleSheet("color: white; font-size: 18px; font-weight: bold; background-color: transparent;");
    currentBookTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_currentBookBtn = new QPushButton("未选择", topWidget);
    m_currentBookBtn->setFixedWidth(180);
    m_currentBookBtn->setStyleSheet(
        "QPushButton {"
        "   color: #FFD700;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   text-decoration: underline;"
        "}"
        );
    m_currentBookBtn->setCursor(Qt::PointingHandCursor);

    QLabel *clickHint = new QLabel("点击切换", topWidget);
    clickHint->setFixedWidth(70);
    clickHint->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 15px; background-color: transparent;");
    clickHint->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    topLayout->addWidget(currentBookTitle);
    topLayout->addWidget(m_currentBookBtn);
    topLayout->addWidget(clickHint);
    topLayout->addStretch();

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->insertWidget(0, topWidget);
    }

    loadBookNames();
    loadSelectedBook();
    loadDataFromFile();
    displayRecords();

    this->setAttribute(Qt::WA_StyledBackground, true);
    updateTopbarStyle(ui->sortTime);

    connect(m_currentBookBtn, &QPushButton::clicked, this, &Record::on_currentBookBtn_clicked);
    connect(ui->sortTime, &QPushButton::clicked, this, &Record::on_sortTime_clicked);
    connect(ui->sortCategory, &QPushButton::clicked, this, &Record::on_sortCategory_clicked);
}

Record::~Record() { delete ui; }

void Record::loadBookNames()
{
    m_bookNames.clear();
    QString filePath = QDir::currentPath() + "/books.json";
    QFile file(filePath);
    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return;

    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        QString bookName = obj["name"].toString();
        if (!bookName.isEmpty()) {
            m_bookNames.append(bookName);
        }
    }
}

void Record::loadSelectedBook()
{
    QString filePath = QDir::currentPath() + "/selected_book.json";
    QFile file(filePath);
    if (!file.exists()) {
        m_currentBookName = "未选择";
        m_selectedRow = -1;
        if (m_currentBookBtn) m_currentBookBtn->setText(m_currentBookName);
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString savedBookName = obj["selectedBookName"].toString();

    loadBookNames();

    if (!savedBookName.isEmpty() && m_bookNames.contains(savedBookName)) {
        m_currentBookName = savedBookName;
        m_selectedRow = m_bookNames.indexOf(savedBookName);
    } else {
        m_currentBookName = "未选择";
        m_selectedRow = -1;
    }

    if (m_currentBookBtn) m_currentBookBtn->setText(m_currentBookName);
}

void Record::saveSelectedBook()
{
    QString filePath = QDir::currentPath() + "/selected_book.json";
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QJsonObject obj;
    obj["selectedRow"] = m_selectedRow;
    obj["selectedBookName"] = m_currentBookName;

    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
}

void Record::loadDataFromFile() {
    m_allRecords.clear();

    if (m_currentBookName.isEmpty() || m_currentBookName == "未选择") {
        return;
    }

    QString fileName = QString("%1_data.txt").arg(m_currentBookName);
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() >= 6) {
            AccountItem item;
            item.date = parts[1].trimmed();
            item.type = parts[2].trimmed();
            item.category = parts[3].trimmed();
            item.amount = parts[4].toDouble();
            item.remark = parts[5].trimmed();
            m_allRecords.append(item);
        }
    }
    file.close();
}

void Record::saveDataToFile() {
    if (m_currentBookName.isEmpty() || m_currentBookName == "未选择") {
        return;
    }

    QString fileName = QString("%1_data.txt").arg(m_currentBookName);
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "错误", "保存到账本文件失败！");
        return;
    }

    QTextStream out(&file);
    for (const auto& item : m_allRecords) {
        out << m_currentBookName << ","
            << item.date << ","
            << item.type << ","
            << item.category << ","
            << item.amount << ","
            << item.remark << "\\n";
    }
    file.close();
}

void Record::displayRecords() {
    ui->listRecords->clear();

    if (m_currentSortType == 0) {
        std::sort(m_allRecords.begin(), m_allRecords.end(), [](const AccountItem& a, const AccountItem& b) {
            return a.date > b.date;
        });
    } else {
        std::sort(m_allRecords.begin(), m_allRecords.end(), [](const AccountItem& a, const AccountItem& b) {
            if (a.category == b.category) return a.date > b.date;
            return a.category < b.category;
        });
    }

    for (int i = 0; i < m_allRecords.size(); ++i) {
        const auto& item = m_allRecords[i];
        QString sign = (item.type == "支出") ? "-" : "+";
        QString displayText = QString("[%1] %2 | %3 %4元 (%5)")
                                  .arg(item.date).arg(item.category).arg(sign)
                                  .arg(QString::number(item.amount, 'f', 2)).arg(item.remark);

        QListWidgetItem *listItem = new QListWidgetItem(ui->listRecords);
        listItem->setSizeHint(QSize(listItem->sizeHint().width(), 45));

        QWidget *rowWidget = new QWidget(this);
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(15, 0, 15, 0);

        QLabel *textLabel = new QLabel(displayText, rowWidget);
        QFont font("Microsoft YaHei", 11);
        textLabel->setFont(font);
        textLabel->setStyleSheet("color: black; background-color: transparent;");

        rowLayout->addWidget(textLabel);
        rowLayout->addStretch();

        QPushButton *editBtn = new QPushButton("修改", rowWidget);
        editBtn->setFixedSize(55, 26);
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setStyleSheet(
            "QPushButton { background-color: #fcfcfc; border: 1px solid #dcdfe6; border-radius: 4px; color: #606266; font-size: 12px; }"
            "QPushButton:hover { background-color: #ecf5ff; color: #409eff; border-color: #c6e2ff; }"
            );

        QPushButton *delBtn = new QPushButton("删除", rowWidget);
        delBtn->setFixedSize(55, 26);
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setStyleSheet(
            "QPushButton { background-color: #ffffff; border: 1px solid #fde2e2; border-radius: 4px; color: #f56c6c; font-size: 12px; }"
            "QPushButton:hover { background-color: #fef0f0; color: #fff; background-color: #f56c6c; }"
            );

        rowLayout->addWidget(editBtn);
        rowLayout->setSpacing(8);
        rowLayout->addWidget(delBtn);

        connect(editBtn, &QPushButton::clicked, this, [this, i]() { on_editRecord_clicked(i); });
        connect(delBtn, &QPushButton::clicked, this, [this, i]() { on_deleteRecord_clicked(i); });

        ui->listRecords->addItem(listItem);
        ui->listRecords->setItemWidget(listItem, rowWidget);
    }
}

void Record::on_deleteRecord_clicked(int index)
{
    if (index < 0 || index >= m_allRecords.size()) return;

    const auto& target = m_allRecords[index];
    QString confirmMsg = QString("确定要删除这条记录吗？\\n\\n[%1] %2 %3元")
                             .arg(target.date).arg(target.category).arg(target.amount);

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除", confirmMsg,
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    m_allRecords.removeAt(index);
    saveDataToFile();
    displayRecords();
}

void Record::on_editRecord_clicked(int index)
{
    if (index < 0 || index >= m_allRecords.size()) return;

    auto& target = m_allRecords[index];

    bool ok;
    double newAmount = QInputDialog::getDouble(this, "修改金额",
                                               QString("请为【%1 - %2】设置新金额：").arg(target.date).arg(target.category),
                                               target.amount, 0.01, 999999.0, 2, &ok);

    if (ok && newAmount > 0) {
        QString newRemark = QInputDialog::getText(this, "修改备注", "修改备注",
                                                  QLineEdit::Normal, target.remark, &ok);

        target.amount = newAmount;
        if (ok) target.remark = newRemark.isEmpty() ? "无" : newRemark.trimmed();

        saveDataToFile();
        displayRecords();
        QMessageBox::information(this, "提示", "修改成功！");
    }
}

void Record::on_sortTime_clicked() {
    m_currentSortType = 0;
    displayRecords();
    updateTopbarStyle(ui->sortTime);
}

void Record::on_sortCategory_clicked() {
    m_currentSortType = 1;
    displayRecords();
    updateTopbarStyle(ui->sortCategory);
}

void Record::updateTopbarStyle(QPushButton* activeBtn)
{
    QString activeStyle = "background-color: #8c1515; color: white; border: none; font-weight: bold;";
    QString inactiveStyle = "background-color: #FFFFFF; color: #333333; border: none; font-weight: bold;";

    if (ui->sortTime == activeBtn) {
        ui->sortTime->setStyleSheet(activeStyle);
    } else {
        ui->sortTime->setStyleSheet(inactiveStyle);
    }

    if (ui->sortCategory == activeBtn) {
        ui->sortCategory->setStyleSheet(activeStyle);
    } else {
        ui->sortCategory->setStyleSheet(inactiveStyle);
    }
}

void Record::on_currentBookBtn_clicked()
{
    loadBookNames();

    if (m_bookNames.isEmpty()) {
        QMessageBox::warning(this, "提示", "没有可用的账本，请先在\"账本管理\"中创建账本");
        return;
    }

    bool ok;
    QString selectedBook = QInputDialog::getItem(this, "切换账本", "请选择要切换的账本：",
                                                 m_bookNames, m_selectedRow, false, &ok);

    if (ok && !selectedBook.isEmpty()) {
        saveDataToFile();

        m_currentBookName = selectedBook;
        m_selectedRow = m_bookNames.indexOf(selectedBook);

        saveSelectedBook();

        if (m_currentBookBtn) m_currentBookBtn->setText(m_currentBookName);

        loadDataFromFile();
        displayRecords();

        QMessageBox::information(this, "切换成功", QString("已切换到账本：%1").arg(selectedBook));
    }
}

void Record::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadSelectedBook();
    loadDataFromFile();
    displayRecords();
}