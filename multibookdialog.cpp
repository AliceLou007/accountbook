#include "multibookdialog.h"
#include <QMessageBox>
#include <QRandomGenerator>
#include <QClipboard>
#include <QGuiApplication>

MultiBookDialog::MultiBookDialog(const QStringList &bookNames, QWidget *parent)
    : MultiBookDialog(bookNames, QMap<QString, QString>(), parent)
{
}

MultiBookDialog::MultiBookDialog(const QStringList &bookNames, const QMap<QString, QString> &inviteCodes, QWidget *parent)
    : QDialog(parent)
    , m_actionType(Invite)
    , m_bookNames(bookNames)
    , m_inviteCodes(inviteCodes)
{
    setupUI();
    setWindowTitle("多人账本");
    setFixedSize(450, 400);
    setModal(true);
    setStyleSheet("background-color: white;");
}

void MultiBookDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // 标题
    m_titleLabel = new QLabel("多人账本", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 22px;"
        "   font-weight: bold;"
        "   color: black;"
        "   padding: 10px;"
        "}"
        );
    mainLayout->addWidget(m_titleLabel);

    // 模式选择按钮
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->setSpacing(20);

    m_inviteBtn = new QPushButton("邀请别人加入", this);
    m_joinBtn = new QPushButton("输入邀请码加入", this);

    QString btnStyle =
        "QPushButton {"
        "   background-color: #f0f0f0;"
        "   color: black;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}";

    QString btnActiveStyle =
        "QPushButton {"
        "   background-color: #8c1515;"
        "   color: white;"
        "   border: 1px solid #8c1515;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #a02020;"
        "}";

    m_inviteBtn->setStyleSheet(btnActiveStyle);
    m_joinBtn->setStyleSheet(btnStyle);

    modeLayout->addWidget(m_inviteBtn);
    modeLayout->addWidget(m_joinBtn);
    mainLayout->addLayout(modeLayout);

    // 分割线
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #e0e0e0; max-height: 1px;");
    mainLayout->addWidget(line);

    // 选择账本（邀请模式）
    QLabel *bookLabel = new QLabel("选择账本：", this);
    bookLabel->setStyleSheet("font-size: 14px; color: black;");
    m_bookCombo = new QComboBox(this);
    m_bookCombo->addItems(m_bookNames);
    m_bookCombo->setStyleSheet(
        "QComboBox {"
        "   padding: 8px;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "   color: black;"
        "}"
        "QComboBox QAbstractItemView {"
        "   color: black;"
        "}"
        );
    mainLayout->addWidget(bookLabel);
    mainLayout->addWidget(m_bookCombo);

    // 邀请码显示区域
    m_inviteCodeLabel = new QLabel(this);
    m_inviteCodeLabel->setAlignment(Qt::AlignCenter);
    m_inviteCodeLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #f8f8f8;"
        "   border: 2px dashed #8c1515;"
        "   border-radius: 8px;"
        "   padding: 20px;"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "   color: black;"
        "   font-family: monospace;"
        "}"
        );
    mainLayout->addWidget(m_inviteCodeLabel);

    // 邀请码输入区域（加入模式）
    QLabel *joinLabel = new QLabel("请输入邀请码：", this);
    joinLabel->setStyleSheet("font-size: 14px; color: black;");
    m_inviteCodeEdit = new QLineEdit(this);
    m_inviteCodeEdit->setPlaceholderText("例如：A3B9F2");
    m_inviteCodeEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 10px;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   font-size: 16px;"
        "   color: black;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #8c1515;"
        "}"
        );
    mainLayout->addWidget(joinLabel);
    mainLayout->addWidget(m_inviteCodeEdit);

    // 提示标签
    m_hintLabel = new QLabel(this);
    m_hintLabel->setWordWrap(true);
    m_hintLabel->setStyleSheet("font-size: 12px; color: black;");
    mainLayout->addWidget(m_hintLabel);

    // 复制按钮（仅邀请模式）
    QPushButton *copyBtn = new QPushButton("复制邀请码", this);
    copyBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #f0f0f0;"
        "   color: black;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #e0e0e0;"
        "}"
        );
    mainLayout->addWidget(copyBtn);

    // 确认按钮
    m_confirmBtn = new QPushButton("确认", this);
    m_confirmBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #8c1515;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #a02020;"
        "}"
        );
    mainLayout->addWidget(m_confirmBtn);

    // 连接信号
    connect(m_inviteBtn, &QPushButton::clicked, this, &MultiBookDialog::onInviteMode);
    connect(m_joinBtn, &QPushButton::clicked, this, &MultiBookDialog::onJoinMode);
    connect(m_confirmBtn, &QPushButton::clicked, this, &MultiBookDialog::onConfirm);
    connect(m_bookCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MultiBookDialog::onBookSelected);
    connect(copyBtn, &QPushButton::clicked, this, [this]() {
        QGuiApplication::clipboard()->setText(m_inviteCodeLabel->text());
        QMessageBox::information(this, "提示", "邀请码已复制到剪贴板");
    });

    // 初始化显示
    updateUI();
    onBookSelected(0);
}

void MultiBookDialog::onInviteMode()
{
    m_actionType = Invite;
    updateUI();
}

void MultiBookDialog::onJoinMode()
{
    m_actionType = Join;
    updateUI();
}

void MultiBookDialog::onBookSelected(int index)
{
    if (index >= 0 && index < m_bookNames.size()) {
        m_selectedBook = m_bookNames[index];
        m_inviteCode = m_inviteCodes.value(m_selectedBook);
        m_inviteCodeLabel->setText(m_inviteCode.isEmpty() ? "确认后生成" : m_inviteCode);
    }
}

void MultiBookDialog::onConfirm()
{
    if (m_actionType == Invite) {
        // 邀请模式：显示邀请码
        accept();
    } else {
        // 加入模式：验证邀请码
        QString inputCode = m_inviteCodeEdit->text().trimmed().toUpper();
        if (inputCode.isEmpty()) {
            QMessageBox::warning(this, "提示", "请输入邀请码");
            return;
        }
        m_inviteCode = inputCode;
        accept();
    }
}

void MultiBookDialog::updateUI()
{
    if (m_actionType == Invite) {
        // 邀请模式
        m_inviteBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #8c1515;"
            "   color: white;"
            "   border: 1px solid #8c1515;"
            "   border-radius: 8px;"
            "   padding: 10px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "}"
            );
        m_joinBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #f0f0f0;"
            "   color: black;"
            "   border: 1px solid #d0d0d0;"
            "   border-radius: 8px;"
            "   padding: 10px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "}"
            );

        m_bookCombo->setVisible(true);
        m_inviteCodeLabel->setVisible(true);
        m_inviteCodeEdit->setVisible(false);
        m_hintLabel->setText("已有多人账本会显示服务器邀请码；普通账本确认后会先创建在线账本");

    } else {
        // 加入模式
        m_joinBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #8c1515;"
            "   color: white;"
            "   border: 1px solid #8c1515;"
            "   border-radius: 8px;"
            "   padding: 10px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "}"
            );
        m_inviteBtn->setStyleSheet(
            "QPushButton {"
            "   background-color: #f0f0f0;"
            "   color: black;"
            "   border: 1px solid #d0d0d0;"
            "   border-radius: 8px;"
            "   padding: 10px;"
            "   font-size: 14px;"
            "   font-weight: bold;"
            "}"
            );

        m_bookCombo->setVisible(false);
        m_inviteCodeLabel->setVisible(false);
        m_inviteCodeEdit->setVisible(true);
        m_hintLabel->setText("输入对方给你的邀请码，即可加入对应的账本");
    }
}

QString MultiBookDialog::generateInviteCode()
{
    const QString characters = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QString code;
    for (int i = 0; i < 6; ++i) {
        int index = QRandomGenerator::global()->bounded(characters.length());
        code.append(characters.at(index));
    }
    return code;
}
