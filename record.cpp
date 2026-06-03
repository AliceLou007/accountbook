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

Record::Record(QWidget *parent) : QWidget(parent), ui(new Ui::Record) {
    ui->setupUi(this);

    // ========== 添加红底白字抬头（只添加这部分） ==========
    QWidget *topWidget = new QWidget(this);
    topWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #8c1515;"
        "   border: none;"
        "}"
        );
    topWidget->setFixedHeight(50);

    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(20, 0, 20, 0);

    QLabel *currentBookTitle = new QLabel("当前账本：", topWidget);
    currentBookTitle->setStyleSheet("color: white; font-size: 14px; font-weight: bold; background-color: transparent;");

    m_currentBookBtn = new QPushButton("未选择，ps:还没有连接上具体的内容，选了明细会消失", topWidget);
    m_currentBookBtn->setStyleSheet(
        "QPushButton {"
        "   color: #FFD700;"
        "   font-size: 14px;"
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
    clickHint->setStyleSheet("color: rgba(255,255,255,0.7); font-size: 11px; background-color: transparent;");

    topLayout->addWidget(currentBookTitle);
    topLayout->addWidget(m_currentBookBtn);
    topLayout->addWidget(clickHint);
    topLayout->addStretch();

    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->insertWidget(0, topWidget);
    }
    // ====================================================

    loadDataFromFile(); // 1. 一出生先去读文件
    displayRecords();   // 2. 展现数据（默认按时间）

    this->setAttribute(Qt::WA_StyledBackground, true);
    updateTopbarStyle(ui->sortTime);

    connect(m_currentBookBtn, &QPushButton::clicked, this, &Record::on_currentBookBtn_clicked);
}

Record::~Record() { delete ui; }

// 从 data.txt 加载数据
void Record::loadDataFromFile() {
    m_allRecords.clear();

    QString fileName = "data.txt";
    if (!m_currentBookName.isEmpty() && m_currentBookName != "未选择，ps:还没有连接上具体的内容，选了明细会消失") {
        fileName = QString("%1_data.txt").arg(m_currentBookName);
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        // 假设你的 data.txt 格式是: 2026-05-31,支出,15.50,餐饮,食堂
        if (parts.size() >= 5) {
            AccountItem item;
            item.date = parts[0].trimmed();
            item.type = parts[1].trimmed();
            item.category = parts[2].trimmed();
            item.amount = parts[3].toDouble();
            item.remark = parts[4].trimmed();
            m_allRecords.append(item);
        }
    }
    file.close();
}

// 核心功能：根据排序规则刷新显示
void Record::displayRecords() {
    ui->listRecords->clear();

    // ✨ 不再读取下拉框，直接看当前状态
    if (m_currentSortType == 0) {
        // 按时间降序
        std::sort(m_allRecords.begin(), m_allRecords.end(), [](const AccountItem& a, const AccountItem& b) {
            return a.date > b.date;
        });
    } else {
        // 按分类排序
        std::sort(m_allRecords.begin(), m_allRecords.end(), [](const AccountItem& a, const AccountItem& b) {
            if (a.category == b.category) return a.date > b.date;
            return a.category < b.category;
        });
    }

    // 渲染显示到 listRecords
    for (const auto& item : m_allRecords) {
        QString sign = (item.type == "支出") ? "-" : "+";
        QString displayText = QString("[%1] %2 | %3 %4元 (%5)")
                                  .arg(item.date).arg(item.category).arg(sign)
                                  .arg(QString::number(item.amount, 'f', 2)).arg(item.remark);

        QListWidgetItem *listItem = new QListWidgetItem(displayText);

        QFont font = listItem->font();
        font.setPointSize(12);

        font.setFamily("Microsoft YaHei");

        listItem->setFont(font);

        listItem->setSizeHint(QSize(listItem->sizeHint().width(), 35));
        ui->listRecords->addItem(listItem);
    }
}

void Record::on_sortTime_clicked() {
    m_currentSortType = 0; // 切换状态为时间
    displayRecords();      // 重新排序并刷新大屏幕 listRecords
    updateTopbarStyle(ui->sortTime);
}

// 点击"按分类排序"按钮
void Record::on_sortCategory_clicked() {
    m_currentSortType = 1; // 切换状态为分类
    displayRecords();      // 重新排序并刷新大屏幕 listRecords
    updateTopbarStyle(ui->sortCategory);
}

void Record::updateTopbarStyle(QPushButton* activeBtn)
{

    QString activeStyle = "background-color: #8c1515; color: white; border: none; font-weight: bold;";
    QString inactiveStyle = "background-color: #FFFFFF; color: #333333; border: none; font-weight: bold;";

    // 时间按钮发牌
    if (ui->sortTime == activeBtn) {
        ui->sortTime->setStyleSheet(activeStyle);
    } else {
        ui->sortTime->setStyleSheet(inactiveStyle);
    }

    // 分类按钮发牌
    if (ui->sortCategory == activeBtn) {
        ui->sortCategory->setStyleSheet(activeStyle);
    } else {
        ui->sortCategory->setStyleSheet(inactiveStyle);
    }
}

// ========== 添加的槽函数实现 ==========
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
        if (!bookName.isEmpty()) m_bookNames.append(bookName);
    }
}

void Record::loadSelectedBook()
{
    QString filePath = QDir::currentPath() + "/selected_book.json";
    QFile file(filePath);
    if (!file.exists()) {
        m_currentBookName = "未选择，ps:还没有连接上具体的内容，选了明细会消失";
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
        m_currentBookName = "未选择，ps:还没有连接上具体的内容，选了明细会消失";
        m_selectedRow = -1;
    }
    if (m_currentBookBtn) m_currentBookBtn->setText(m_currentBookName);
}

void Record::updateCurrentBookDisplay()
{
    loadBookNames();
    loadSelectedBook();
    loadDataFromFile();
    displayRecords();
}

void Record::on_currentBookBtn_clicked()
{
    loadBookNames();
    if (m_bookNames.isEmpty()) {
        QMessageBox::warning(this, "提示", "没有可用的账本，请先在\"账本管理\"中创建账本");
        return;
    }
    bool ok;
    QString selectedBook = QInputDialog::getItem(this, "切换账本", "请选择要切换的账本：", m_bookNames, m_selectedRow, false, &ok);
    if (ok && !selectedBook.isEmpty()) {
        m_currentBookName = selectedBook;
        m_selectedRow = m_bookNames.indexOf(selectedBook);
        if (m_currentBookBtn) m_currentBookBtn->setText(m_currentBookName);
        loadDataFromFile();
        displayRecords();
        QMessageBox::information(this, "切换成功", QString("已切换到账本：%1").arg(selectedBook));
    }
}