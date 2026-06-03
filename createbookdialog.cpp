#include "createbookdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>

CreateBookDialog::CreateBookDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void CreateBookDialog::setupUI()
{
    setWindowTitle("创建新账本");
    setFixedSize(400, 300);
    setModal(true);  // 设置为模态对话框

    // 设置白色背景
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    setPalette(palette);
    setAutoFillBackground(true);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // ========== 账本名称（必填） ==========
    QLabel *nameLabel = new QLabel("账本名称：", this);
    nameLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #333333;");

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("请输入账本名称（必填）");
    m_nameEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "   background-color: white;"
        "   color:black;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #8c1515;"
        "   color:black;"
        "}"
        );
    connect(m_nameEdit, &QLineEdit::textChanged, this, &CreateBookDialog::validateInput);

    // 必填标记
    QLabel *requiredLabel = new QLabel("* 必填", this);
    requiredLabel->setStyleSheet("color: #8c1515; font-size: 11px;");

    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(nameLabel);
    nameLayout->addStretch();
    nameLayout->addWidget(requiredLabel);

    // ========== 备注 ==========
    QLabel *remarkLabel = new QLabel("备注：", this);
    remarkLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #333333; margin-top: 10px;");

    m_remarkEdit = new QTextEdit(this);
    m_remarkEdit->setPlaceholderText("请输入备注（选填）");
    m_remarkEdit->setMaximumHeight(100);
    m_remarkEdit->setStyleSheet(
        "QTextEdit {"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "   background-color: white;"
        "   color:black;"
        "}"
        "QTextEdit:focus {"
        "   border-color: #8c1515;"
        "   color:black;"
        "}"
        );

    // 错误提示标签
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #8c1515; font-size: 11px;");
    m_errorLabel->setVisible(false);

    // ========== 按钮 ==========
    m_btnConfirm = new QPushButton("创建", this);
    m_btnCancel = new QPushButton("取消", this);

    QString btnStyle =
        "QPushButton {"
        "   background-color: rgba(140, 21, 21, 0.08);"
        "   color: #444444;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   padding: 8px 20px;"
        "   font-size: 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(140, 21, 21, 0.15);"
        "   color: #8c1515;"
        "   border-color: #8c1515;"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(140, 21, 21, 0.25);"
        "}";

    m_btnConfirm->setStyleSheet(btnStyle);
    m_btnCancel->setStyleSheet(btnStyle);

    // 初始状态，确认按钮不可用（因为账本名称为空）
    m_btnConfirm->setEnabled(false);
    m_btnConfirm->setStyleSheet(btnStyle + "QPushButton:disabled { background-color: #f0f0f0; color: #999999; border-color: #d0d0d0; }");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnConfirm);
    btnLayout->addWidget(m_btnCancel);
    btnLayout->addStretch();

    // ========== 组装布局 ==========
    mainLayout->addLayout(nameLayout);
    mainLayout->addWidget(m_nameEdit);
    mainLayout->addWidget(remarkLabel);
    mainLayout->addWidget(m_remarkEdit);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);

    // 连接信号槽
    connect(m_btnConfirm, &QPushButton::clicked, this, &CreateBookDialog::onConfirm);
    connect(m_btnCancel, &QPushButton::clicked, this, &CreateBookDialog::onCancel);
}

void CreateBookDialog::validateInput()
{
    // 检查账本名称是否为空
    bool isValid = !m_nameEdit->text().trimmed().isEmpty();
    m_btnConfirm->setEnabled(isValid);

    if (m_nameEdit->text().trimmed().isEmpty()) {
        m_errorLabel->setText("请输入账本名称");
        m_errorLabel->setVisible(true);
    } else {
        m_errorLabel->setVisible(false);
    }
}

void CreateBookDialog::onConfirm()
{
    QString bookName = m_nameEdit->text().trimmed();

    if (bookName.isEmpty()) {
        m_errorLabel->setText("账本名称不能为空");
        m_errorLabel->setVisible(true);
        return;
    }

    // 验证通过，接受对话框
    accept();
}

void CreateBookDialog::onCancel()
{
    reject();
}

QString CreateBookDialog::getBookName() const
{
    return m_nameEdit->text().trimmed();
}

QString CreateBookDialog::getRemark() const
{
    return m_remarkEdit->toPlainText().trimmed();
}