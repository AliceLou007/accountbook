#include "editbookdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

EditBookDialog::EditBookDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void EditBookDialog::setupUI()
{
    setWindowTitle("修改账本信息");
    setFixedSize(400, 350);
    setModal(true);

    // 设置白色背景
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    setPalette(palette);
    setAutoFillBackground(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // 账本名称
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
        "   color: black;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #8c1515;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #999999;"
        "}"
        );
    connect(m_nameEdit, &QLineEdit::textChanged, this, &EditBookDialog::validateInput);

    QLabel *requiredLabel = new QLabel("* 必填", this);
    requiredLabel->setStyleSheet("color: #8c1515; font-size: 11px;");

    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(nameLabel);
    nameLayout->addStretch();
    nameLayout->addWidget(requiredLabel);

    // 备注
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
        "   color: black;"
        "}"
        "QTextEdit:focus {"
        "   border-color: #8c1515;"
        "}"
        );

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #8c1515; font-size: 11px;");
    m_errorLabel->setVisible(false);

    // 按钮
    m_btnConfirm = new QPushButton("保存", this);
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

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnConfirm);
    btnLayout->addWidget(m_btnCancel);
    btnLayout->addStretch();

    mainLayout->addLayout(nameLayout);
    mainLayout->addWidget(m_nameEdit);
    mainLayout->addWidget(remarkLabel);
    mainLayout->addWidget(m_remarkEdit);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);

    connect(m_btnConfirm, &QPushButton::clicked, this, &EditBookDialog::onConfirm);
    connect(m_btnCancel, &QPushButton::clicked, this, &EditBookDialog::onCancel);
}

void EditBookDialog::setBookInfo(const QString& name, const QString& remark)
{
    m_oldName = name;
    m_nameEdit->setText(name);
    m_remarkEdit->setText(remark);
}

QString EditBookDialog::getBookName() const
{
    return m_nameEdit->text().trimmed();
}

QString EditBookDialog::getRemark() const
{
    return m_remarkEdit->toPlainText().trimmed();
}

void EditBookDialog::validateInput()
{
    bool isValid = !m_nameEdit->text().trimmed().isEmpty();
    m_btnConfirm->setEnabled(isValid);

    if (m_nameEdit->text().trimmed().isEmpty()) {
        m_errorLabel->setText("账本名称不能为空");
        m_errorLabel->setVisible(true);
    } else {
        m_errorLabel->setVisible(false);
    }
}

void EditBookDialog::onConfirm()
{
    QString bookName = m_nameEdit->text().trimmed();

    if (bookName.isEmpty()) {
        m_errorLabel->setText("账本名称不能为空");
        m_errorLabel->setVisible(true);
        return;
    }

    accept();
}

void EditBookDialog::onCancel()
{
    reject();
}