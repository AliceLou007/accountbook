#include "tagmanagerdialog.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QInputDialog>
#include <QDir>
#include "userdata.h"

TagManagerDialog::TagManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadTagsFromFile();
}

void TagManagerDialog::setupUI()
{
    setWindowTitle("标签分类设置");
    setFixedSize(800, 500);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel *titleLabel = new QLabel("标签管理", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #8c1515;"
        "   color: white;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   padding: 10px;"
        "   border-radius: 5px;"
        "}"
        );
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(45);
    mainLayout->addWidget(titleLabel);

    // 两个列表的水平布局
    QHBoxLayout *listsLayout = new QHBoxLayout();

    // 支出标签区域
    QWidget *expenseWidget = new QWidget(this);
    QVBoxLayout *expenseLayout = new QVBoxLayout(expenseWidget);

    QLabel *expenseTitle = new QLabel("支出标签", expenseWidget);
    expenseTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "   padding: 5px;"
        "   background-color: rgba(140, 21, 21, 0.1);"
        "   border-radius: 3px;"
        "}"
        );
    expenseTitle->setAlignment(Qt::AlignCenter);

    m_expenseList = new QListWidget(expenseWidget);
    m_expenseList->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   font-size: 12px;"
        "   color: #000000;"
        "}"
        "QListWidget::item {"
        "   padding: 8px;"
        "   color: #000000;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(140, 21, 21, 0.15);"
        "   color: #000000;"
        "}"
        );

    QHBoxLayout *expenseBtnLayout = new QHBoxLayout();
    m_btnExpenseAdd = new QPushButton("添加", expenseWidget);
    m_btnExpenseDelete = new QPushButton("删除", expenseWidget);

    // 移除编辑按钮
    // m_btnExpenseEdit 不再创建

    QString btnStyle =
        "QPushButton {"
        "   background-color: #f0f0f0;"
        "   color: #333333;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   padding: 5px 15px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #d0d0d0;"
        "}";

    m_btnExpenseAdd->setStyleSheet(btnStyle);
    m_btnExpenseDelete->setStyleSheet(btnStyle);

    expenseBtnLayout->addWidget(m_btnExpenseAdd);
    expenseBtnLayout->addWidget(m_btnExpenseDelete);

    expenseLayout->addWidget(expenseTitle);
    expenseLayout->addWidget(m_expenseList);
    expenseLayout->addLayout(expenseBtnLayout);

    // 收入标签区域
    QWidget *incomeWidget = new QWidget(this);
    QVBoxLayout *incomeLayout = new QVBoxLayout(incomeWidget);

    QLabel *incomeTitle = new QLabel("收入标签", incomeWidget);
    incomeTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "   padding: 5px;"
        "   background-color: rgba(140, 21, 21, 0.1);"
        "   border-radius: 3px;"
        "}"
        );
    incomeTitle->setAlignment(Qt::AlignCenter);

    m_incomeList = new QListWidget(incomeWidget);
    m_incomeList->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   font-size: 12px;"
        "   color: #000000;"
        "}"
        "QListWidget::item {"
        "   padding: 8px;"
        "   color: #000000;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(140, 21, 21, 0.15);"
        "   color: #000000;"
        "}"
        );

    QHBoxLayout *incomeBtnLayout = new QHBoxLayout();
    m_btnIncomeAdd = new QPushButton("添加", incomeWidget);
    m_btnIncomeDelete = new QPushButton("删除", incomeWidget);

    // 移除编辑按钮
    // m_btnIncomeEdit 不再创建

    m_btnIncomeAdd->setStyleSheet(btnStyle);
    m_btnIncomeDelete->setStyleSheet(btnStyle);

    incomeBtnLayout->addWidget(m_btnIncomeAdd);
    incomeBtnLayout->addWidget(m_btnIncomeDelete);

    incomeLayout->addWidget(incomeTitle);
    incomeLayout->addWidget(m_incomeList);
    incomeLayout->addLayout(incomeBtnLayout);

    listsLayout->addWidget(expenseWidget);
    listsLayout->addWidget(incomeWidget);
    listsLayout->setSpacing(30);

    mainLayout->addLayout(listsLayout);

    // 底部按钮
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QPushButton *btnClose = new QPushButton("关闭", this);
    btnClose->setStyleSheet(btnStyle);
    btnClose->setFixedWidth(100);

    bottomLayout->addStretch();
    bottomLayout->addWidget(btnClose);
    bottomLayout->addStretch();

    mainLayout->addLayout(bottomLayout);

    // 连接信号（只连接添加和删除）
    connect(m_btnExpenseAdd, &QPushButton::clicked, this, &TagManagerDialog::onExpenseAdd);
    connect(m_btnExpenseDelete, &QPushButton::clicked, this, &TagManagerDialog::onExpenseDelete);
    connect(m_btnIncomeAdd, &QPushButton::clicked, this, &TagManagerDialog::onIncomeAdd);
    connect(m_btnIncomeDelete, &QPushButton::clicked, this, &TagManagerDialog::onIncomeDelete);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

void TagManagerDialog::initDefaultTags()
{
    m_expenseTags = QStringList() << "餐饮" << "购物" << "日用" << "交通"
                                  << "娱乐" << "医疗" << "其他";
    m_incomeTags = QStringList() << "工资" << "生活费" << "奖学金"
                                 << "兼职" << "理财" << "其他";
}

void TagManagerDialog::loadTagsFromFile()
{
    QFile file(UserData::tagsFile());
    if (!file.exists()) {
        initDefaultTags();
        saveTagsToFile();
        return;
    }

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (doc.isObject()) {
            QJsonObject obj = doc.object();

            if (obj.contains("expenseTags") && obj["expenseTags"].isArray()) {
                m_expenseTags.clear();
                QJsonArray expenseArray = obj["expenseTags"].toArray();
                for (const QJsonValue &value : expenseArray) {
                    if (value.isString()) {
                        m_expenseTags << value.toString();
                    }
                }
            }

            if (obj.contains("incomeTags") && obj["incomeTags"].isArray()) {
                m_incomeTags.clear();
                QJsonArray incomeArray = obj["incomeTags"].toArray();
                for (const QJsonValue &value : incomeArray) {
                    if (value.isString()) {
                        m_incomeTags << value.toString();
                    }
                }
            }
        }
        file.close();
    }

    // 更新显示
    m_expenseList->clear();
    m_expenseList->addItems(m_expenseTags);
    m_incomeList->clear();
    m_incomeList->addItems(m_incomeTags);
}

void TagManagerDialog::saveTagsToFile()
{
    QJsonObject obj;
    QJsonArray expenseArray;
    QJsonArray incomeArray;

    for (const QString &tag : m_expenseTags) {
        expenseArray.append(tag);
    }

    for (const QString &tag : m_incomeTags) {
        incomeArray.append(tag);
    }

    obj["expenseTags"] = expenseArray;
    obj["incomeTags"] = incomeArray;

    QJsonDocument doc(obj);
    QFile file(UserData::tagsFile());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void TagManagerDialog::onExpenseAdd()
{
    bool ok;
    QString newTag = QInputDialog::getText(this, "添加支出标签",
                                           "请输入新标签名称：",
                                           QLineEdit::Normal, "", &ok);
    if (ok && !newTag.isEmpty()) {
        if (m_expenseTags.contains(newTag)) {
            QMessageBox::warning(this, "警告", "标签已存在！");
            return;
        }
        m_expenseTags.append(newTag);
        m_expenseList->addItem(newTag);
        saveTagsToFile();
    }
}

void TagManagerDialog::onExpenseDelete()
{
    QListWidgetItem *item = m_expenseList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "警告", "请先选择要删除的标签！");
        return;
    }

    QString tag = item->text();

    // 检查是否为"其他"标签
    if (tag == "其他") {
        QMessageBox::warning(this, "警告", "\"其他\"标签不可删除！");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
                                                              QString("确定要删除标签 \"%1\" 吗？\\\\n删除后，所有使用此标签的记录将自动改为\"其他\"！").arg(tag),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_expenseTags.removeOne(tag);
        delete item;
        saveTagsToFile();
    }
}

void TagManagerDialog::onIncomeAdd()
{
    bool ok;
    QString newTag = QInputDialog::getText(this, "添加收入标签",
                                           "请输入新标签名称：",
                                           QLineEdit::Normal, "", &ok);
    if (ok && !newTag.isEmpty()) {
        if (m_incomeTags.contains(newTag)) {
            QMessageBox::warning(this, "警告", "标签已存在！");
            return;
        }
        m_incomeTags.append(newTag);
        m_incomeList->addItem(newTag);
        saveTagsToFile();
    }
}

void TagManagerDialog::onIncomeDelete()
{
    QListWidgetItem *item = m_incomeList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "警告", "请先选择要删除的标签！");
        return;
    }

    QString tag = item->text();

    // 检查是否为"其他"标签
    if (tag == "其他") {
        QMessageBox::warning(this, "警告", "\"其他\"标签不可删除！");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
                                                              QString("确定要删除标签 \"%1\" 吗？\\\\n删除后，所有使用此标签的记录将自动改为\"其他\"！").arg(tag),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_incomeTags.removeOne(tag);
        delete item;
        saveTagsToFile();
    }
}

QStringList TagManagerDialog::getExpenseTags() const
{
    return m_expenseTags;
}

QStringList TagManagerDialog::getIncomeTags() const
{
    return m_incomeTags;
}
