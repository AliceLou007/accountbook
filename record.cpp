#include "record.h"
#include "ui_record.h"
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <QHBoxLayout>
#include "editrecorddialog.h"
#include <QDateTime>
#include <QFileInfo>
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
#include <QPixmap>
#include <QDialog>
#include <QPushButton>
#include "userdata.h"

Record::Record(QWidget *parent) : QWidget(parent), ui(new Ui::Record), m_currentSortType(0), m_selectedRow(-1) {
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

    QHBoxLayout *topLayout = new QHBoxLayout(topWidget);
    topLayout->setContentsMargins(150, 10, 150, 0);
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

    QLabel *clickHint = new QLabel( topWidget);
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

    connect(m_currentBookBtn, &QPushButton::clicked, this, &Record::on_currentBookBtn_clicked);
    connect(ui->sortTime, &QPushButton::clicked, this, &Record::on_sortTime_clicked);

    connect(ui->sortCategory, &QPushButton::clicked, this, [this]() {
        if (ui->sortCategory->menu()) {
            QPoint pos = ui->sortCategory->mapToGlobal(QPoint(0, ui->sortCategory->height()));
            ui->sortCategory->menu()->exec(pos);
        }
    });

    updateCategoryMenu();
}

Record::~Record() { delete ui; }

void Record::loadBookNames()
{
    m_bookNames.clear();
    QString filePath = UserData::booksFile();
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
    QString filePath = UserData::selectedBookFile();
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
    QString filePath = UserData::selectedBookFile();
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

    QFile file(UserData::recordFile(m_currentBookName));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "文件不存在或无法打开";
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(",");
        AccountItem item;
        item.book = parts.size() > 0 ? parts[0].trimmed() : "";
        item.date = parts.size() > 1 ? parts[1].trimmed() : "";
        item.type = parts.size() > 2 ? parts[2].trimmed() : "";
        item.category = parts.size() > 3 ? parts[3].trimmed() : "";
        item.amount = parts.size() > 4 ? parts[4].toDouble() : 0;
        item.remark = parts.size() > 5 ? parts[5].trimmed() : "";
        item.imagePath = parts.size() > 6 ? parts[6].trimmed() : "";
        m_allRecords.append(item);
    }
    file.close();
}

void Record::saveDataToFile() {
    if (m_currentBookName.isEmpty() || m_currentBookName == "未选择") {
        return;
    }

    QFile file(UserData::recordFile(m_currentBookName));

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
            << item.remark << ","
            << item.imagePath << "\n";
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
        // 如果有图片，添加查看图片按钮
        if (!item.imagePath.isEmpty() && QFile::exists(item.imagePath)) {
            QPushButton *viewImageBtn = new QPushButton("查看图片", rowWidget);
            viewImageBtn->setFixedSize(75, 26);
            viewImageBtn->setCursor(Qt::PointingHandCursor);
            viewImageBtn->setStyleSheet(
                "QPushButton { background-color: #fcfcfc; border: 1px solid #dcdfe6; border-radius: 4px; color: #409eff; font-size: 12px; }"
                "QPushButton:hover { background-color: #ecf5ff; color: #409eff; border-color: #c6e2ff; }"
                );
            rowLayout->addWidget(viewImageBtn);
            connect(viewImageBtn, &QPushButton::clicked, this, [this, i]() { on_viewImage_clicked(i); });
        }
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

    // 创建编辑对话框，传入原有的图片路径
    EditRecordDialog dialog(target.date, target.type, target.category,
                            target.amount, target.remark, target.imagePath, this);

    if (dialog.exec() == QDialog::Accepted) {
        // 更新记录
        target.date = dialog.getDate();
        target.type = dialog.getType();
        target.category = dialog.getCategory();
        target.amount = dialog.getAmount();
        target.remark = dialog.getRemark();

        // 处理图片：如果用户选择了新图片，则复制并更新路径
        if (dialog.isImageChanged()) {
            QString newImagePath = dialog.getImagePath();
            if (!newImagePath.isEmpty()) {
                // 复制新图片到账本目录
                QString savedImagePath = copyImageToBookForRecord(newImagePath, m_currentBookName);
                if (!savedImagePath.isEmpty()) {
                    target.imagePath = savedImagePath;
                }
            } else if (target.imagePath.isEmpty()) {
                target.imagePath = "";
            }
        }

        saveDataToFile();
        emit dataChanged();
        displayRecords();

        QMessageBox::information(this, "成功", "记录修改成功！");
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

        // 添加查看图片按钮
        if (!item.imagePath.isEmpty() && QFile::exists(item.imagePath)) {
            QPushButton *viewImageBtn = new QPushButton("查看图片", rowWidget);
            viewImageBtn->setFixedSize(75, 26);
            viewImageBtn->setCursor(Qt::PointingHandCursor);
            viewImageBtn->setStyleSheet(
                "QPushButton { background-color: #fcfcfc; border: 1px solid #dcdfe6; border-radius: 4px; color: #409eff; font-size: 12px; }"
                "QPushButton:hover { background-color: #ecf5ff; color: #409eff; border-color: #c6e2ff; }"
                );
            rowLayout->addWidget(viewImageBtn);
            connect(viewImageBtn, &QPushButton::clicked, this, [this, i]() { on_viewImage_clicked(i); });
        }

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
        checkAndFixCategories();
        displayRecords();
        updateCategoryMenu();
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
    checkAndFixCategories();
    updateCategoryMenu();
    displayRecords();
}

void Record::updateCategoryMenu()
{
    QStringList incomeCategories;
    QStringList expenseCategories;

    QString tagFilePath = UserData::tagsFile();
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

    if (incomeCategories.isEmpty()) {
        incomeCategories = {"工资", "生活费", "奖学金", "兼职", "理财", "其他"};
    }
    if (expenseCategories.isEmpty()) {
        expenseCategories = {"餐饮", "购物", "日用", "交通", "娱乐", "医疗", "其他"};
    }

    if (ui->sortCategory->menu()) {
        delete ui->sortCategory->menu();
    }

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

    QAction *incomeTitle = mainMenu->addAction("━━━ 收入 ━━━");
    incomeTitle->setEnabled(false);

    for (const QString &category : incomeCategories) {
        QAction *act = mainMenu->addAction(category);
        connect(act, &QAction::triggered, this, [this, category]() {
            filterAndDisplayRecords(category);
        });
    }

    mainMenu->addSeparator();

    QAction *expenseTitle = mainMenu->addAction("━━━ 支出 ━━━");
    expenseTitle->setEnabled(false);

    for (const QString &category : expenseCategories) {
        QAction *act = mainMenu->addAction(category);
        connect(act, &QAction::triggered, this, [this, category]() {
            filterAndDisplayRecords(category);
        });
    }

    mainMenu->addSeparator();

    QAction *showAllAct = mainMenu->addAction("显示全部");
    connect(showAllAct, &QAction::triggered, this, [this]() {
        on_sortTime_clicked();
    });

    ui->sortCategory->setMenu(mainMenu);
}

void Record::checkAndFixCategories()
{
    QStringList validIncomeTags;
    QStringList validExpenseTags;

    QString tagFilePath = UserData::tagsFile();
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

    if (validIncomeTags.isEmpty()) {
        validIncomeTags = {"工资", "生活费", "奖学金", "兼职", "理财", "其他"};
    }
    if (validExpenseTags.isEmpty()) {
        validExpenseTags = {"餐饮", "购物", "日用", "交通", "娱乐", "医疗", "其他"};
    }

    QStringList validTags = validIncomeTags + validExpenseTags;
    validTags.removeDuplicates();

    bool needSave = false;

    for (AccountItem &item : m_allRecords) {
        if (!validTags.contains(item.category)) {
            item.category = "其他";
            needSave = true;
        }
    }

    if (needSave) {
        saveDataToFile();
        QMessageBox::information(this, "提示", "检测到已删除的标签，相关记录已自动改为\"其他\"。");
    }
}

void Record::loadTagsFromFile()
{
    updateCategoryMenu();
}

// ========== 查看图片功能 ==========
void Record::on_viewImage_clicked(int index)
{
    if (index < 0 || index >= m_allRecords.size()) return;

    const auto& item = m_allRecords[index];
    QString imagePath = item.imagePath;

    if (imagePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "该记录没有关联图片！");
        return;
    }

    if (!QFile::exists(imagePath)) {
        QMessageBox::warning(this, "提示", QString("图片文件不存在：\n%1").arg(imagePath));
        return;
    }

    // 创建图片查看对话框
    QDialog *imageDialog = new QDialog(this);
    imageDialog->setWindowTitle(QString("图片 - %1 %2 %3元")
                                    .arg(item.date)
                                    .arg(item.category)
                                    .arg(item.amount, 0, 'f', 2));
    imageDialog->setModal(true);
    imageDialog->setMinimumSize(600, 500);
    imageDialog->setStyleSheet("background-color: #ffffff;");

    QVBoxLayout *layout = new QVBoxLayout(imageDialog);

    // 加载图片
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        QMessageBox::warning(imageDialog, "错误", "无法加载图片！");
        delete imageDialog;
        return;
    }

    // 缩放图片
    QLabel *imageLabel = new QLabel(imageDialog);
    QPixmap scaledPixmap = pixmap.scaled(550, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(scaledPixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background-color: #f5f5f5; border: 1px solid #ddd; padding: 10px;");

    // 信息标签
    QLabel *infoLabel = new QLabel(QString("日期: %1 | 类型: %2 | 分类: %3 | 金额: %4元 | 备注: %5")
                                       .arg(item.date)
                                       .arg(item.type)
                                       .arg(item.category)
                                       .arg(item.amount, 0, 'f', 2)
                                       .arg(item.remark), imageDialog);
    infoLabel->setStyleSheet("color: #666666; font-size: 12px; padding: 5px;");
    infoLabel->setWordWrap(true);
    infoLabel->setAlignment(Qt::AlignCenter);

    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("关闭", imageDialog);
    closeBtn->setFixedSize(100, 32);
    closeBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #8c1515;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #a01c1c;"
        "}"
        );

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    btnLayout->addStretch();

    layout->addWidget(imageLabel);
    layout->addWidget(infoLabel);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, imageDialog, &QDialog::accept);

    imageDialog->exec();
    delete imageDialog;
}
// 复制图片到账本目录
QString Record::copyImageToBookForRecord(const QString &imagePath, const QString &bookName)
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
