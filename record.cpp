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
#include <QMenu>
#include <QDebug>

Record::Record(QWidget *parent) : QWidget(parent), ui(new Ui::Record), m_currentSortType(0), m_selectedRow(-1) {
    ui->setupUi(this);

    // 禁用自动连接（避免冲突）
    // 不要在这里添加 on_sortCategory_clicked 之类的槽函数

    // ========== 添加红底白字抬头 ==========
    QWidget *topWidget = new QWidget(this);
    topWidget->setStyleSheet(
        "QWidget {"
        "   background-color: #8c1515;"
        "   border: none;"
        "}"
        );
    topWidget->setFixedHeight(70);

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
    checkAndFixCategories();
    displayRecords();

    updateTopbarStyle(ui->sortTime);

    // 手动连接信号（避免自动连接冲突）
    connect(m_currentBookBtn, &QPushButton::clicked, this, &Record::on_currentBookBtn_clicked);
    connect(ui->sortTime, &QPushButton::clicked, this, &Record::on_sortTime_clicked);

    // 关键：手动连接分类按钮的点击事件来弹出菜单
    connect(ui->sortCategory, &QPushButton::clicked, this, [this]() {
        qDebug() << "分类按钮被点击";
        if (ui->sortCategory->menu()) {
            QPoint pos = ui->sortCategory->mapToGlobal(QPoint(0, ui->sortCategory->height()));
            qDebug() << "弹出菜单位置:" << pos;
            ui->sortCategory->menu()->exec(pos);
        } else {
            qDebug() << "菜单为空！";
        }
    });

    // 创建分类菜单
    updateCategoryMenu();
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
        qDebug() << "没有选中账本";
        return;
    }

    QString fileName = QString("%1_data.txt").arg(m_currentBookName);
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "文件不存在或无法打开";
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() >= 6) {
            AccountItem item;
            item.book = parts[0].trimmed();
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
        out << item.book << ","
            << item.date << ","
            << item.type << ","
            << item.category << ","
            << item.amount << ","
            << item.remark << "\n";
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
    QString confirmMsg = QString("确定要删除这条记录吗？\n[%1] %2 %3元")
                             .arg(target.date).arg(target.category).arg(target.amount);

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除", confirmMsg,
                                                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    m_allRecords.removeAt(index);
    saveDataToFile();
    emit dataChanged();
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
        emit dataChanged();
        displayRecords();
    }
}

void Record::on_sortTime_clicked() {
    m_currentSortType = 0;
    ui->sortCategory->setText("分类筛选");
    displayRecords();
    updateTopbarStyle(ui->sortTime);
}

void Record::filterAndDisplayRecords(const QString &selectedCategory)
{
    m_currentSortType = 1;
    ui->sortCategory->setText("分类: " + selectedCategory);
    updateTopbarStyle(ui->sortCategory);

    ui->listRecords->clear();

    for (int i = 0; i < m_allRecords.size(); ++i) {
        const auto& item = m_allRecords[i];

        if (item.category != selectedCategory) {
            continue;
        }

        QString sign = (item.type == "支出") ? "-" : "+";
        QString displayText = QString("[%1] %2 | %3 %4元 (%5)")
                                  .arg(item.date)
                                  .arg(item.category)
                                  .arg(sign)
                                  .arg(QString::number(item.amount, 'f', 2))
                                  .arg(item.remark);

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

void Record::updateTopbarStyle(QPushButton* activeBtn)
{
    QString activeStyle = "background-color: #8c1515; color: white; border: none; font-weight: bold;";
    QString inactiveStyle = "background-color: #FFFFFF; color: #333333; border: none; font-weight: bold;";

    ui->sortTime->setStyleSheet((ui->sortTime == activeBtn) ? activeStyle : inactiveStyle);
    ui->sortCategory->setStyleSheet((ui->sortCategory == activeBtn) ? activeStyle : inactiveStyle);
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
        checkAndFixCategories();  // 检查并修复无效标签
        displayRecords();
        updateCategoryMenu();     // 更新分类菜单
    }
    emit bookChanged();
}

void Record::updateCurrentBookDisplay()
{
    if (m_currentBookBtn) {
        m_currentBookBtn->setText(m_currentBookName);
    }
}

void Record::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadSelectedBook();
    loadDataFromFile();
    checkAndFixCategories();  // 检查并修复无效标签
    updateCategoryMenu();     // 更新分类菜单
    displayRecords();
}

// ========== 标签管理相关函数 ==========

void Record::updateCategoryMenu()
{
    qDebug() << "=== updateCategoryMenu 被调用 ===";

    QStringList incomeCategories;
    QStringList expenseCategories;

    QString tagFilePath = QDir::currentPath() + "/tags.json";
    QFile file(tagFilePath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            QJsonObject obj = doc.object();

            if (obj.contains("incomeTags") && obj["incomeTags"].isArray()) {
                QJsonArray incomeArray = obj["incomeTags"].toArray();
                for (const QJsonValue &value : incomeArray) {
                    if (value.isString()) {
                        incomeCategories << value.toString();
                    }
                }
            }

            if (obj.contains("expenseTags") && obj["expenseTags"].isArray()) {
                QJsonArray expenseArray = obj["expenseTags"].toArray();
                for (const QJsonValue &value : expenseArray) {
                    if (value.isString()) {
                        expenseCategories << value.toString();
                    }
                }
            }
        }
        file.close();
    }

    // 使用默认标签
    if (incomeCategories.isEmpty()) {
        incomeCategories = {"工资", "生活费", "奖学金", "兼职", "理财", "其他"};
    }
    if (expenseCategories.isEmpty()) {
        expenseCategories = {"餐饮", "购物", "日用", "交通", "娱乐", "医疗", "其他"};
    }

    qDebug() << "收入标签:" << incomeCategories;
    qDebug() << "支出标签:" << expenseCategories;

    // 删除旧的菜单
    if (ui->sortCategory->menu()) {
        delete ui->sortCategory->menu();
    }

    // 创建菜单
    QMenu *mainMenu = new QMenu(this);
    mainMenu->setStyleSheet(
        "QMenu {"
        "   background-color: white;"
        "   border: 1px solid #d0d0d0;"
        "   padding: 5px;"
        "}"
        "QMenu::item {"
        "   padding: 5px 20px;"
        "   color: black;"
        "}"
        "QMenu::item:selected {"
        "   background-color: rgba(140, 21, 21, 0.1);"
        "   color: #8c1515;"
        "}"
        );

    // 添加收入标签
    QAction *incomeTitle = mainMenu->addAction("━━━ 收入 ━━━");
    incomeTitle->setEnabled(false);

    for (const QString &category : incomeCategories) {
        QAction *act = mainMenu->addAction(category);
        connect(act, &QAction::triggered, this, [this, category]() {
            qDebug() << "选择了收入分类:" << category;
            filterAndDisplayRecords(category);
        });
    }

    mainMenu->addSeparator();

    // 添加支出标签
    QAction *expenseTitle = mainMenu->addAction("━━━ 支出 ━━━");
    expenseTitle->setEnabled(false);

    for (const QString &category : expenseCategories) {
        QAction *act = mainMenu->addAction(category);
        connect(act, &QAction::triggered, this, [this, category]() {
            qDebug() << "选择了支出分类:" << category;
            filterAndDisplayRecords(category);
        });
    }

    mainMenu->addSeparator();

    // 显示全部
    QAction *showAllAct = mainMenu->addAction("显示全部");
    connect(showAllAct, &QAction::triggered, this, [this]() {
        qDebug() << "选择了显示全部";
        on_sortTime_clicked();
    });

    // 设置菜单
    ui->sortCategory->setMenu(mainMenu);
    qDebug() << "菜单已设置，标签数量:" << (incomeCategories.size() + expenseCategories.size());
}

void Record::checkAndFixCategories()
{
    qDebug() << "=== checkAndFixCategories 被调用 ===";

    QStringList validIncomeTags;
    QStringList validExpenseTags;

    QString tagFilePath = QDir::currentPath() + "/tags.json";
    QFile file(tagFilePath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            QJsonObject obj = doc.object();

            if (obj.contains("incomeTags") && obj["incomeTags"].isArray()) {
                QJsonArray incomeArray = obj["incomeTags"].toArray();
                for (const QJsonValue &value : incomeArray) {
                    if (value.isString()) {
                        validIncomeTags << value.toString();
                    }
                }
            }

            if (obj.contains("expenseTags") && obj["expenseTags"].isArray()) {
                QJsonArray expenseArray = obj["expenseTags"].toArray();
                for (const QJsonValue &value : expenseArray) {
                    if (value.isString()) {
                        validExpenseTags << value.toString();
                    }
                }
            }
        }
        file.close();
    }

    // 使用默认标签
    if (validIncomeTags.isEmpty()) {
        validIncomeTags = {"工资", "生活费", "奖学金", "兼职", "理财", "其他"};
    }
    if (validExpenseTags.isEmpty()) {
        validExpenseTags = {"餐饮", "购物", "日用", "交通", "娱乐", "医疗", "其他"};
    }

    QStringList validTags = validIncomeTags + validExpenseTags;
    validTags.removeDuplicates();

    qDebug() << "有效标签列表:" << validTags;

    bool needSave = false;

    // 将无效标签改为"其他"
    for (AccountItem &item : m_allRecords) {
        if (!validTags.contains(item.category)) {
            qDebug() << "发现无效标签:" << item.category << "，已改为\"其他\"";
            item.category = "其他";
            needSave = true;
        }
    }

    if (needSave) {
        saveDataToFile();
        QMessageBox::information(this, "提示", "检测到已删除的标签，相关记录已自动改为\"其他\"。");
    } else {
        qDebug() << "所有标签都有效，无需修改";
    }
}

void Record::loadTagsFromFile()
{
    // 此函数保留供外部调用
    updateCategoryMenu();
}