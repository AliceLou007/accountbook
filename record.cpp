#include "record.h"
#include "ui_record.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>

Record::Record(QWidget *parent) : QWidget(parent), ui(new Ui::Record) {
    ui->setupUi(this);

    loadDataFromFile(); // 1. 一出生先去读文件
    displayRecords();   // 2. 展现数据（默认按时间）
}

Record::~Record() { delete ui; }

// 从 data.txt 加载数据
void Record::loadDataFromFile() {
    m_allRecords.clear();
    QFile file("data.txt");
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
        ui->listRecords->addItem(displayText);
    }
}

void Record::on_btnSortTime_clicked() {
    m_currentSortType = 0; // 切换状态为时间
    displayRecords();      // 重新排序并刷新大屏幕 listRecords
}

// 点击“按分类排序”按钮
void Record::on_btnSortCategory_clicked() {
    m_currentSortType = 1; // 切换状态为分类
    displayRecords();      // 重新排序并刷新大屏幕 listRecords
}