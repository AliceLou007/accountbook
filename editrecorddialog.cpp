#include "editrecorddialog.h"

EditRecordDialog::EditRecordDialog(const QString &date, const QString &type,
                                   const QString &category, double amount,
                                   const QString &remark, QWidget *parent)
    : QDialog(parent)
    , m_originalDate(date)
    , m_originalType(type)
    , m_originalCategory(category)
    , m_originalAmount(amount)
    , m_originalRemark(remark)
{
    setupUI();
    loadCategoriesFromTags();

    // 设置初始值
    m_dateEdit->setDate(QDate::fromString(date, "yyyy-MM-dd"));
    int typeIndex = m_typeCombo->findText(type);
    if (typeIndex >= 0) m_typeCombo->setCurrentIndex(typeIndex);
    m_amountSpin->setValue(amount);
    m_remarkEdit->setText(remark);

    // 根据类型设置分类下拉框
    onTypeChanged(type);
    int categoryIndex = m_categoryCombo->findText(category);
    if (categoryIndex >= 0) m_categoryCombo->setCurrentIndex(categoryIndex);
}

void EditRecordDialog::setupUI()
{
    setWindowTitle("编辑记录");
    setFixedSize(400, 350);
    setModal(true);

    setStyleSheet(
        "QDialog {"
        "   background-color: #ffffff;"
        "   border-radius: 12px;"
        "}"
        "QLabel {"
        "   font-size: 14px;"
        "   color: #000000;"
        "}"
        "QDateEdit, QComboBox, QLineEdit, QDoubleSpinBox {"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 6px;"
        "   padding: 6px;"
        "   background-color: #fcfcfc;"
        "   font-size: 14px;"
        "   color: #000000;"
        "}"
        "QDateEdit:focus, QComboBox:focus, QLineEdit:focus, QDoubleSpinBox:focus {"
        "   border: 1.5px solid #8c1515;"
        "}"
        "QComboBox QAbstractItemView {"
        "   color: #000000;"
        "}"
        "QPushButton {"
        "   background-color: #f0f0f0;"
        "   color: #000000;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 6px;"
        "   padding: 8px 20px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}"
        "QPushButton#okBtn {"
        "   background-color: #8c1515;"
        "   color: #ffffff;"
        "   border: none;"
        "}"
        "QPushButton#okBtn:hover {"
        "   background-color: #a01c1c;"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 日期
    QHBoxLayout *dateLayout = new QHBoxLayout();
    QLabel *dateLabel = new QLabel("日期：", this);
    dateLabel->setFixedWidth(80);
    dateLabel->setStyleSheet("color: #000000;");
    m_dateEdit = new QDateEdit(this);
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("yyyy-MM-dd");
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(m_dateEdit);
    mainLayout->addLayout(dateLayout);

    // 类型
    QHBoxLayout *typeLayout = new QHBoxLayout();
    QLabel *typeLabel = new QLabel("类型：", this);
    typeLabel->setFixedWidth(80);
    typeLabel->setStyleSheet("color: #000000;");
    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem("支出");
    m_typeCombo->addItem("收入");
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(m_typeCombo);
    mainLayout->addLayout(typeLayout);

    // 分类
    QHBoxLayout *categoryLayout = new QHBoxLayout();
    QLabel *categoryLabel = new QLabel("分类：", this);
    categoryLabel->setFixedWidth(80);
    categoryLabel->setStyleSheet("color: #000000;");
    m_categoryCombo = new QComboBox(this);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(m_categoryCombo);
    mainLayout->addLayout(categoryLayout);

    // 金额
    QHBoxLayout *amountLayout = new QHBoxLayout();
    QLabel *amountLabel = new QLabel("金额：", this);
    amountLabel->setFixedWidth(80);
    amountLabel->setStyleSheet("color: #000000;");
    m_amountSpin = new QDoubleSpinBox(this);
    m_amountSpin->setRange(0.01, 99999999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setPrefix("￥ ");
    amountLayout->addWidget(amountLabel);
    amountLayout->addWidget(m_amountSpin);
    mainLayout->addLayout(amountLayout);

    // 备注
    QHBoxLayout *remarkLayout = new QHBoxLayout();
    QLabel *remarkLabel = new QLabel("备注：", this);
    remarkLabel->setFixedWidth(80);
    remarkLabel->setStyleSheet("color: #000000;");
    m_remarkEdit = new QLineEdit(this);
    remarkLayout->addWidget(remarkLabel);
    remarkLayout->addWidget(m_remarkEdit);
    mainLayout->addLayout(remarkLayout);

    mainLayout->addStretch();

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_okBtn = new QPushButton("确定", this);
    m_okBtn->setObjectName("okBtn");
    m_okBtn->setFixedWidth(100);
    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setFixedWidth(100);
    btnLayout->addWidget(m_okBtn);
    btnLayout->addSpacing(20);
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // 连接信号
    connect(m_typeCombo, &QComboBox::currentTextChanged, this, &EditRecordDialog::onTypeChanged);
    connect(m_okBtn, &QPushButton::clicked, this, &EditRecordDialog::onOkClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &EditRecordDialog::onCancelClicked);
}

void EditRecordDialog::loadCategoriesFromTags()
{
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

    if (expenseTags.isEmpty()) {
        expenseTags = {"餐饮", "购物", "日用", "交通", "娱乐", "医疗", "其他"};
    }
    if (incomeTags.isEmpty()) {
        incomeTags = {"工资", "生活费", "奖学金", "兼职", "理财", "其他"};
    }

    m_expenseTags = expenseTags;
    m_incomeTags = incomeTags;
}

void EditRecordDialog::onTypeChanged(const QString &type)
{
    m_categoryCombo->clear();
    if (type == "支出") {
        m_categoryCombo->addItems(m_expenseTags);
    } else {
        m_categoryCombo->addItems(m_incomeTags);
    }
}

void EditRecordDialog::onOkClicked()
{
    if (m_amountSpin->value() <= 0) {
        QMessageBox::warning(this, "提示", "金额必须大于0！");
        return;
    }

    if (m_categoryCombo->currentText().isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择分类！");
        return;
    }

    accept();
}

void EditRecordDialog::onCancelClicked()
{
    reject();
}

QString EditRecordDialog::getDate() const
{
    return m_dateEdit->date().toString("yyyy-MM-dd");
}

QString EditRecordDialog::getType() const
{
    return m_typeCombo->currentText();
}

QString EditRecordDialog::getCategory() const
{
    return m_categoryCombo->currentText();
}

double EditRecordDialog::getAmount() const
{
    return m_amountSpin->value();
}

QString EditRecordDialog::getRemark() const
{
    QString remark = m_remarkEdit->text().trimmed();
    return remark.isEmpty() ? "无" : remark;
}