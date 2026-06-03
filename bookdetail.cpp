#include "bookdetail.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDate>
#include <QPalette>

BookDetail::BookDetail(const QString &bookName, const QString &remark, const QStringList &members, QWidget *parent)
    : QWidget(parent)
    , m_bookName(bookName)
    , m_remark(remark)
    , m_members(members)
{
    setupUI();
    loadRecords();
}

BookDetail::~BookDetail()
{
}

void BookDetail::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 强制设置白色背景
    setAutoFillBackground(true);
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, Qt::white);
    pal.setColor(QPalette::WindowText, Qt::black);
    this->setPalette(pal);

    // 强制刷新整个窗口
    this->update();
    this->repaint();
}

void BookDetail::setupUI()
{
    setWindowTitle("账本详情 - " + m_bookName);
    setFixedSize(700, 900);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ========== 标题 ==========
    QLabel *titleLabel = new QLabel("账本详情", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #8c1515;"
        "   color: white;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   padding: 15px;"
        "   border-radius: 5px;"
        "}"
        );
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFixedHeight(60);

    // ========== 信息区域 ==========
    QFrame *infoFrame = new QFrame(this);
    infoFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #fafafa;"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 8px;"
        "}"
        );
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);

    // 账本名称
    m_nameLabel = new QLabel(this);
    m_nameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333; padding: 5px; background-color: transparent;");
    m_nameLabel->setText(QString("📒 账本名称：%1").arg(m_bookName));

    // 备注
    m_remarkLabel = new QLabel(this);
    m_remarkLabel->setStyleSheet("font-size: 12px; color: #666666; padding: 5px; background-color: transparent;");
    m_remarkLabel->setText(QString("📝 备注：%1").arg(m_remark.isEmpty() ? "无" : m_remark));
    m_remarkLabel->setWordWrap(true);

    // 参与人员
    QString membersStr = m_members.isEmpty() ? "无" : m_members.join("、");
    m_membersLabel = new QLabel(this);
    m_membersLabel->setStyleSheet("font-size: 12px; color: #666666; padding: 5px; background-color: transparent;");
    m_membersLabel->setText(QString("👥 参与人员：%1").arg(membersStr));

    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_remarkLabel);
    infoLayout->addWidget(m_membersLabel);
    infoLayout->addStretch();

    // ========== 只有返回按钮 ==========
    QHBoxLayout *btnLayout = new QHBoxLayout();

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
        "}";

    m_btnBack = new QPushButton("← 返回", this);
    m_btnBack->setStyleSheet(btnStyle);
    m_btnBack->setFixedWidth(100);

    btnLayout->addStretch();
    btnLayout->addWidget(m_btnBack);
    btnLayout->addStretch();

    // ========== 账本条例表格 ==========
    QLabel *recordsTitle = new QLabel("账本条例列表", this);
    recordsTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333; margin-top: 10px;");

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(QStringList() << "日期" << "类型" << "金额" << "分类" << "备注");
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(true);
    m_table->setFixedHeight(500);
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_table->setStyleSheet(
        "QTableWidget {"
        "   background-color: white;"
        "   gridline-color: #e0e0e0;"
        "   outline: 0px;"
        "   color:#333333;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: rgba(140, 21, 21, 0.12);"
        "}"
        "QHeaderView::section {"
        "   background-color: #fafafa;"
        "   padding: 8px;"
        "   border: none;"
        "   border-bottom: 1px solid #e0e0e0;"
        "   font-weight: bold;"
        "   color: #333333;"
        "}"
        );

    m_table->setColumnWidth(0, 100);
    m_table->setColumnWidth(1, 80);
    m_table->setColumnWidth(2, 100);
    m_table->setColumnWidth(3, 100);
    m_table->horizontalHeader()->setStretchLastSection(true);

    // ========== 布局 ==========
    mainLayout->addWidget(titleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(infoFrame);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(btnLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(recordsTitle);
    mainLayout->addSpacing(5);
    mainLayout->addWidget(m_table);
    mainLayout->addStretch();

    setLayout(mainLayout);

    // 只连接返回按钮
    connect(m_btnBack, &QPushButton::clicked, this, &BookDetail::onBack);
}

void BookDetail::loadRecords()
{
    m_table->setRowCount(0);

    struct Record {
        QString date;
        QString type;
        QString amount;
        QString category;
        QString remark;
    };

    QList<Record> records = {
                             {"2024-01-15", "支出", "128.00", "餐饮", "午饭"},
                             {"2024-01-16", "收入", "5000.00", "工资", "1月工资"},
                             {"2024-01-17", "支出", "45.00", "交通", "地铁充值"},
                             {"2024-01-18", "支出", "267.00", "购物", "超市购物"},
                             };

    for (int i = 0; i < records.size(); ++i) {
        m_table->insertRow(i);

        QTableWidgetItem *dateItem = new QTableWidgetItem(records[i].date);
        dateItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *typeItem = new QTableWidgetItem(records[i].type);
        typeItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *amountItem = new QTableWidgetItem("¥" + records[i].amount);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QTableWidgetItem *categoryItem = new QTableWidgetItem(records[i].category);
        categoryItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *remarkItem = new QTableWidgetItem(records[i].remark);

        m_table->setItem(i, 0, dateItem);
        m_table->setItem(i, 1, typeItem);
        m_table->setItem(i, 2, amountItem);
        m_table->setItem(i, 3, categoryItem);
        m_table->setItem(i, 4, remarkItem);
    }

    m_table->verticalHeader()->setDefaultSectionSize(45);
}

void BookDetail::onBack()
{
    emit backToManage();  // 发送信号而不是直接关闭
    this->hide();  // 隐藏当前页面
}