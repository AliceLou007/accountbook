#include "widget.h"
#include "ui_widget.h"
#include "manage.h"
#include "addone.h"
#include "record.h"
#include "budget.h"
#include "datachartwidget.h"
#include <QDebug>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QCoreApplication>
#include <QPixmap>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QPainterPath>
#include "userdata.h"

Widget::Widget(const QString &userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_userId(userId)
{
    ui->setupUi(this);

    this->setFixedSize(950, 850);
    ui->stackedWidget->setCurrentIndex(0);

    ui->label_9->setGeometry(30, 22, 86, 32);
    ui->label_9->setAlignment(Qt::AlignCenter);
    ui->label_9->setStyleSheet(
        "QLabel {"
        "   background-color: transparent;"
        "   color: #8c1515;"
        "   border: none;"
        "   font-size: 14px;"
        "   font-weight: 600;"
        "}"
        );

    ui->lblBudgetAmount->setGeometry(128, 14, 250, 46);
    ui->lblBudgetAmount->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->lblBudgetAmount->setStyleSheet(
        "QLabel {"
        "   color: #1a1a1a;"
        "   font-size: 22px;"
        "   font-weight: 600;"
        "   letter-spacing: 0px;"
        "}"
        );

    ui->lblBudgetLeftAmount->setGeometry(30, 66, 650, 34);
    ui->lblBudgetLeftAmount->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->lblBudgetLeftAmount->setWordWrap(false);
    ui->lblBudgetLeftAmount->setStyleSheet(
        "QLabel {"
        "   background-color: #fbf7f7;"
        "   color: #6b4a4a;"
        "   border: 1px solid #f0dddd;"
        "   border-radius: 10px;"
        "   padding-left: 14px;"
        "   padding-right: 14px;"
        "   font-size: 13px;"
        "}"
        );

    m_recordPage = new Record(this);
    ui->stackedWidget->insertWidget(1, m_recordPage);

    m_chartPage = new datachartwidget(this);          // 实例化图表页面
    ui->stackedWidget->insertWidget(2, m_chartPage);   // 强行插入到序号 2 的位置

    m_budgetPage = new Budget(this);
    ui->stackedWidget->insertWidget(3, m_budgetPage);

    m_managePage = new Manage(this);
    ui->stackedWidget->insertWidget(4, m_managePage);

    connect(m_recordPage, &Record::dataChanged, this, [this](){
        updateHomeUi(currentViewingMonth); // 主页随明细页变化
        m_chartPage->loaddata();
        if (m_chartPage) {
            m_chartPage->setbookname("");
        }
    });
    connect(m_recordPage, &Record::bookChanged, this, [this](){
        updateHomeUi(currentViewingMonth); //主页随明细页刷新
        m_chartPage->loaddata();
        if (m_chartPage) {
            m_chartPage->setbookname("");
        }
    });

    currentViewingMonth = QDate::currentDate().toString("yyyy-MM");
    updateHomeUi(currentViewingMonth);
    updateSidebarStyle(ui->btnHome);

    // ========== 创建左上角用户信息区域（头像 + 姓名 + 性别） ==========
    // 创建一个容器 Widget
    m_userInfoWidget = new QWidget(this);
    m_userInfoWidget->setFixedSize(130, 130);
    m_userInfoWidget->move(25, 20);
    m_userInfoWidget->setStyleSheet("background-color: transparent;");

    // 垂直布局
    QVBoxLayout *userInfoLayout = new QVBoxLayout(m_userInfoWidget);
    userInfoLayout->setContentsMargins(0, 0, 0, 0);
    userInfoLayout->setSpacing(5);

    // 头像按钮容器
    m_avatarBtn = new QPushButton(m_userInfoWidget);
    m_avatarBtn->setFixedSize(80, 80);
    m_avatarBtn->setCursor(Qt::PointingHandCursor);
    m_avatarBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   border-radius: 40px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(0, 0, 0, 30);"
        "   border-radius: 40px;"
        "}"
        );

    // 头像标签
    m_avatarLabel = new QLabel(m_avatarBtn);
    m_avatarLabel->setFixedSize(80, 80);
    m_avatarLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #f0f0f0;"
        "   border: 2px solid #8c1515;"
        "   border-radius: 40px;"
        "}"
        );
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setText("👤");
    m_avatarLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #f0f0f0;"
        "   border: 2px solid #8c1515;"
        "   border-radius: 40px;"
        "   font-size: 40px;"
        "}"
        );

    // 设置头像按钮的布局
    QVBoxLayout *avatarLayout = new QVBoxLayout(m_avatarBtn);
    avatarLayout->addWidget(m_avatarLabel);
    avatarLayout->setContentsMargins(0, 0, 0, 0);

    // 姓名标签
    m_nameLabel = new QLabel(m_userInfoWidget);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet(
        "QLabel {"
        "   color: #333333;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   background-color: transparent;"
        "}"
        );

    // 性别标签
    m_genderLabel = new QLabel(m_userInfoWidget);
    m_genderLabel->setAlignment(Qt::AlignCenter);
    m_genderLabel->setStyleSheet(
        "QLabel {"
        "   color: #8c1515;"
        "   font-size: 12px;"
        "   background-color: transparent;"
        "}"
        );

    // 添加到布局
    userInfoLayout->addWidget(m_avatarBtn, 0, Qt::AlignHCenter);
    userInfoLayout->addWidget(m_nameLabel);
    userInfoLayout->addWidget(m_genderLabel);

    // 连接头像点击信号
    connect(m_avatarBtn, &QPushButton::clicked, this, &Widget::onAvatarClicked);

    // 加载用户信息
    loadUserProfile();
    updateUserInfoDisplay();

    // 大红按钮点击事件
    connect(ui->btnOpenAdd, &QPushButton::clicked, this, [=](){
        QWidget *mask = new QWidget(this);
        mask->setGeometry(this->rect());
        mask->setStyleSheet("background-color: rgba(0, 0, 0, 100);");
        mask->show();

        AddOne dialog(this);
        dialog.setFixedSize(450, 400);
        int x = this->x() + (this->width() - dialog.width()) / 2;
        int y = this->y() + (this->height() - dialog.height()) / 2;
        dialog.move(x, y);

        if (dialog.exec() == QDialog::Accepted) {
            updateHomeUi(currentViewingMonth);
        }

        mask->close();
        delete mask;
    });
}

Widget::~Widget()
{
    saveUserProfile();
    delete ui;
}

// 只要一显示就刷新页面
void Widget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateHomeUi(currentViewingMonth);

    qDebug() << "主窗口已显示，首页数据已自动同步刷新！";
}

void Widget::loadAndCalculateAllData()
{
    allMonthsData.clear();

    // ================= 1. 先读取 selected_book.json 获取账本名 =================
    QString jsonPath = UserData::selectedBookFile();
    QFile jsonFile(jsonPath);

    if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray jsonData = jsonFile.readAll();
        jsonFile.close();

        // 解析 JSON 文档
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject jsonObj = jsonDoc.object();
            if (jsonObj.contains("selectedBookName")) {
                // 动态更新成员变量 m_bookName
                m_bookName = jsonObj.value("selectedBookName").toString();
            }
        }
    } else {
        qDebug() << "无法打开 selected_book.json 文件，将使用默认或当前的 m_bookName";
    }

    // 如果 JSON 读取失败且 m_bookName 为空，给一个默认值防止程序崩溃
    if (m_bookName.isEmpty()) {
        m_bookName = "我的账本";
    }

    QString filePath = UserData::recordFile(m_bookName);

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList tokens = line.split(",");

        if (tokens.size() >= 6) {
            QString qBook = tokens[0];
            QString qDate = tokens[1];
            QString qType = tokens[2];
            QString qCategory = tokens[3];
            double amount = tokens[4].toDouble();
            QString qComment = tokens[5];

            QString yearMonthKey = qDate.left(7).replace("/", "-");

            if (qType == "收入") {
                allMonthsData[yearMonthKey].income += amount;
            } else if (qType == "支出") {
                allMonthsData[yearMonthKey].outcome += amount;
            }
            allMonthsData[yearMonthKey].count++;
        }
    }
    file.close();

    for (auto it = allMonthsData.begin(); it != allMonthsData.end(); ++it) {
        it.value().balance = it.value().income - it.value().outcome;
    }
}

void Widget::updateHomeUi(const QString &yearMonth)
{
    // 1. 先加载基础账本数据（此函数内部会更新 m_bookName）
    loadAndCalculateAllData();

    // 2. 动态读取当月预算 JSON 文件
    double budget = 0.0;
    QString budgetPath = UserData::budgetFile(yearMonth, m_bookName);
    QFile budgetFile(budgetPath);

    if (budgetFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(budgetFile.readAll());
        budgetFile.close();
        budget = doc.object()["budget"].toDouble();
    }

    // 3. 如果当月没有任何记账数据
    if (!allMonthsData.contains(yearMonth)) {
        ui->lblIncomeAmount->setText("￥ 0.00");
        ui->lblOutcomeAmount->setText("￥ 0.00");
        ui->lblBalanceAmount->setText("￥ 0.00");
        ui->lblCount->setText("0 笔");

        // 主页预算标签也要更新显示
        ui->lblBudgetAmount->setText("￥ " + QString::number(budget, 'f', 2));
        return;
    }

    // 4. 如果有记账数据，正常渲染主页
    const MonthStat &stat = allMonthsData[yearMonth];

    ui->lblIncomeAmount->setText("￥ " + QString::number(stat.income, 'f', 2));
    ui->lblOutcomeAmount->setText("￥ " + QString::number(stat.outcome, 'f', 2));
    ui->lblBalanceAmount->setText("￥ " + QString::number(stat.balance, 'f', 2));
    ui->lblCount->setText(QString::number(stat.count) + " 笔");

    // 显示当月预算
    ui->lblBudgetAmount->setText("￥ " + QString::number(budget, 'f', 2));

    double budgetLeft = budget - stat.outcome;
    double leftPercent = 0.0;

    // ==================== 核心修改：精准判断预算文件是否存在 ====================
    bool hasBudgetFile = QFile::exists(budgetPath);

    // 计算剩余百分比
    if (budget > 0) {
        leftPercent = (budgetLeft / budget) * 100.0;
    } else {
        leftPercent = 0.0;
    }

    // 拼接百分比字符串
    QString percentStr = QString::number(leftPercent, 'f', 1) + "%";

    // ==================== 动态提示词与颜色控制 ====================
    if (!hasBudgetFile) {
        // 情况 0：预算文件不存在
        QString hint = QString("提示：该账本（%1）当月（%2）暂无预算，请先设置预算").arg(m_bookName, yearMonth);
        ui->lblBudgetLeftAmount->setText(hint);
        ui->lblBudgetLeftAmount->setStyleSheet("background-color: #fff7f7; color: #9E2A2B; border: 1px solid #f1d7d7; border-radius: 10px; padding-left: 14px; padding-right: 14px; font-weight: 600; font-size: 13px;");
    }
    else if (budget <= 0) {
        // 情况 0.5：文件存在，但预算额度被用户设置为了 0
        ui->lblBudgetLeftAmount->setText("当前月预算额度为 0.00，请合理规划用度。");
        ui->lblBudgetLeftAmount->setStyleSheet("background-color: #f8f8f8; color: #555555; border: 1px solid #eeeeee; border-radius: 10px; padding-left: 14px; padding-right: 14px; font-size: 13px;");
    }
    else if (leftPercent > 30.0) {
        // 情况 1：剩余预算大于 30%（安全状态）
        QString hint = QString("剩余预算 %1。手中有粮心中不慌，合理规划，细水长流。").arg(percentStr);
        ui->lblBudgetLeftAmount->setText(hint);
        ui->lblBudgetLeftAmount->setStyleSheet("background-color: #f3faf4; color: #2E7D32; border: 1px solid #d7ead9; border-radius: 10px; padding-left: 14px; padding-right: 14px; font-weight: 600; font-size: 13px;");
    }
    else if (leftPercent <= 30.0 && leftPercent >= 0.0) {
        // 情况 2：剩余预算在 0% ~ 30% 之间（警告状态）
        QString hint = QString("仅剩 %1！一粥一饭当思来处不易，预算将尽，宜克制消费。").arg(percentStr);
        ui->lblBudgetLeftAmount->setText(hint);
        ui->lblBudgetLeftAmount->setStyleSheet("background-color: #fff7f2; color: #D32F2F; border: 1px solid #f3d6cb; border-radius: 10px; padding-left: 14px; padding-right: 14px; font-weight: 600; font-size: 13px;");
    }
    else {
        // 情况 3：已经超支（leftPercent < 0）
        QString hint = QString("已超支 %1！用度无度，仓库已虚，请立即停止不必要支出。").arg(percentStr.remove("-"));
        ui->lblBudgetLeftAmount->setText(hint);
        ui->lblBudgetLeftAmount->setStyleSheet("background-color: #fff4f4; color: #9E2A2B; border: 1px solid #edcccc; border-radius: 10px; padding-left: 14px; padding-right: 14px; font-weight: 700; font-size: 13px;");
    }
}

void Widget::on_btnHome_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
    updateSidebarStyle(ui->btnHome);
    updateHomeUi(currentViewingMonth);
}
void Widget::on_btnHistory_clicked() {
    ui->stackedWidget->setCurrentWidget(m_recordPage);
    updateSidebarStyle(ui->btnHistory);
}
void Widget::on_btnCount_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    updateSidebarStyle(ui->btnCount);
    if (m_chartPage) {
        m_chartPage->loaddata();
    }
    if (m_chartPage) {
        m_chartPage->setbookname("");
    }
}
void Widget::on_btnBudget_clicked() {
    ui->stackedWidget->setCurrentIndex(3);
    updateSidebarStyle(ui->btnBudget);
}
void Widget::on_btnManagement_clicked() {
    ui->stackedWidget->setCurrentIndex(4);
    updateSidebarStyle(ui->btnManagement);
}

void Widget::updateSidebarStyle(QPushButton* activeBtn)
{
    QList<QPushButton*> sidebarBtns = {
        ui->btnHome,
        ui->btnHistory,
        ui->btnCount,
        ui->btnBudget,
        ui->btnManagement
    };

    QString activeStyle = R"(
        QPushButton {
            background-color: rgba(140, 21, 21, 0.85);
            color: white;
            font-size: 14px;
            font-weight: bold;
            text-align: left;
            padding-left: 30px;
            border-left: 3px solid #ff9999;
            border-radius: 0px;
        }
        QPushButton:hover {
            background-color: #8c1515;
        }
    )";

    QString inactiveStyle = R"(
        QPushButton {
            color: #444444;
            font-size: 14px;
            font-weight: bold;
            text-align: left;
            padding-left: 30px;
            background-color: rgba(255, 255, 255, 0.75);
            border: none;
        }
        QPushButton:hover {
            background-color: rgba(140, 21, 21, 0.12);
            color: #8c1515;
        }
    )";

    for (QPushButton* btn : sidebarBtns) {
        if (btn == activeBtn) {
            btn->setStyleSheet(activeStyle);
        } else {
            btn->setStyleSheet(inactiveStyle);
        }
    }
}

void Widget::loadUserProfile()
{
    if (m_userId.isEmpty()) {
        m_userId = "10001";
    }

    QString loginUserId = m_userId;
    QString filePath = userProfilePath();
    QFile file(filePath);

    if (!file.exists()) {
        m_userId = loginUserId;
        m_userName = loginUserId;
        m_userAge = 18;
        m_userGender = "保密";
        m_userSignature = "这个人很懒，什么都没写~";
        m_userAvatar = QPixmap();
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject obj = doc.object();
    m_userId = obj["userId"].toString(loginUserId);
    if (m_userId.isEmpty()) {
        m_userId = loginUserId;
    }
    m_userName = obj["userName"].toString(m_userId);
    if (m_userName.isEmpty()) {
        m_userName = m_userId;
    }
    m_userAge = obj["userAge"].toInt(18);
    m_userGender = obj["userGender"].toString("保密");
    m_userSignature = obj["userSignature"].toString("这个人很懒，什么都没写~");
    m_userAvatar = QPixmap();

    // 加载头像
    QString avatarPath = obj["avatarPath"].toString();
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        m_userAvatar.load(avatarPath);
    }
}

void Widget::saveUserProfile()
{
    QString filePath = userProfilePath();
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QJsonObject obj;
    obj["userId"] = m_userId;
    obj["userName"] = m_userName;
    obj["userAge"] = m_userAge;
    obj["userGender"] = m_userGender;
    obj["userSignature"] = m_userSignature;

    // 保存头像路径
    if (!m_userAvatar.isNull()) {
        QString avatarPath = userAvatarPath();
        m_userAvatar.save(avatarPath);
        obj["avatarPath"] = avatarPath;
    }

    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
}

QString Widget::userProfilePath() const
{
    QString safeUserId = m_userId;
    if (safeUserId.isEmpty()) {
        safeUserId = "default";
    }

    for (QChar &ch : safeUserId) {
        if (!ch.isLetterOrNumber() && ch != '_' && ch != '-') {
            ch = '_';
        }
    }

    Q_UNUSED(safeUserId)
    return UserData::profileFile();
}

QString Widget::userAvatarPath() const
{
    QString safeUserId = m_userId;
    if (safeUserId.isEmpty()) {
        safeUserId = "default";
    }

    for (QChar &ch : safeUserId) {
        if (!ch.isLetterOrNumber() && ch != '_' && ch != '-') {
            ch = '_';
        }
    }

    Q_UNUSED(safeUserId)
    return UserData::avatarFile();
}

void Widget::updateUserInfoDisplay()
{
    // 更新头像显示
    if (!m_userAvatar.isNull()) {
        // 头像已经是圆形的，直接显示
        m_avatarLabel->setPixmap(m_userAvatar);
        m_avatarLabel->setText("");
        m_avatarLabel->setStyleSheet(
            "QLabel {"
            "   border: 2px solid #8c1515;"
            "   border-radius: 40px;"
            "   background-color: white;"
            "}"
            );
    } else {
        m_avatarLabel->setText("??");
        m_avatarLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #f0f0f0;"
            "   border: 2px solid #8c1515;"
            "   border-radius: 40px;"
            "   font-size: 40px;"
            "}"
            );
    }

    // 更新姓名
    QString displayName = m_userName;
    if (displayName.length() > 8) {
        displayName = displayName.left(6) + "...";
    }
    m_nameLabel->setText(displayName);

    // 更新性别显示（带图标）
    QString genderText;
    if (m_userGender == "男") {
        genderText = "♂ 男";
    } else if (m_userGender == "女") {
        genderText = "♀ 女";
    } else {
        genderText = "? 保密";
    }
    m_genderLabel->setText(genderText);
}

void Widget::updateAvatarDisplay()
{
    // 更新头像显示（调用新的统一方法）
    updateUserInfoDisplay();
}

void Widget::onAvatarClicked()
{
    QMenu menu(this);

    QAction *editProfileAction = menu.addAction("✏️ 编辑个人信息");
    QAction *changeAvatarAction = menu.addAction("🖼️ 更换头像");
    menu.addSeparator();
    QAction *logoutAction = menu.addAction("🚪 退出登录");

    connect(editProfileAction, &QAction::triggered, this, &Widget::onEditProfile);
    connect(changeAvatarAction, &QAction::triggered, this, &Widget::onChangeAvatar);
    connect(logoutAction, &QAction::triggered, this, &Widget::onLogout);

    // 在头像位置显示菜单
    QPoint pos = m_avatarBtn->mapToGlobal(QPoint(0, m_avatarBtn->height()));
    menu.exec(pos);
}

void Widget::onEditProfile()
{
    QDialog dialog(this);
    dialog.setWindowTitle("编辑个人信息");
    dialog.setFixedSize(400, 550);
    dialog.setStyleSheet("background-color: white;");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);
    layout->setContentsMargins(30, 30, 30, 30);

    // ID（只读显示）
    QHBoxLayout *idLayout = new QHBoxLayout();
    QLabel *idLabel = new QLabel("ID：");
    idLabel->setFixedWidth(80);
    idLabel->setStyleSheet("font-size: 14px; color: #333333;");
    QLineEdit *idEdit = new QLineEdit(m_userId);
    idEdit->setReadOnly(true);
    idEdit->setStyleSheet("QLineEdit { color: black; background-color: #f5f5f5; padding: 6px; border: 1px solid #d0d0d0; border-radius: 4px; }");
    idLayout->addWidget(idLabel);
    idLayout->addWidget(idEdit);

    // 昵称
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("昵称：");
    nameLabel->setFixedWidth(80);
    nameLabel->setStyleSheet("font-size: 14px; color: #333333;");
    QLineEdit *nameEdit = new QLineEdit(m_userName);
    nameEdit->setPlaceholderText("请输入昵称");
    nameEdit->setStyleSheet("QLineEdit { color: black; padding: 6px; border: 1px solid #d0d0d0; border-radius: 4px; }");  // 添加 color: black;
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);

    // 年龄
    QHBoxLayout *ageLayout = new QHBoxLayout();
    QLabel *ageLabel = new QLabel("年龄：");
    ageLabel->setFixedWidth(80);
    ageLabel->setStyleSheet("font-size: 14px; color: #333333;");
    QLineEdit *ageEdit = new QLineEdit(QString::number(m_userAge));
    ageEdit->setPlaceholderText("请输入年龄");
    ageEdit->setStyleSheet("QLineEdit { color: black; padding: 6px; border: 1px solid #d0d0d0; border-radius: 4px; }");  // 添加 color: black;
    ageLayout->addWidget(ageLabel);
    ageLayout->addWidget(ageEdit);

    // 性别
    QHBoxLayout *genderLayout = new QHBoxLayout();
    QLabel *genderLabel = new QLabel("性别：");
    genderLabel->setFixedWidth(80);
    genderLabel->setStyleSheet("font-size: 14px; color: #333333;");
    QComboBox *genderCombo = new QComboBox();
    genderCombo->addItems({"保密", "男", "女"});
    int index = genderCombo->findText(m_userGender);
    if (index >= 0) genderCombo->setCurrentIndex(index);
    genderCombo->setStyleSheet("QComboBox { color: black; padding: 6px; border: 1px solid #d0d0d0; border-radius: 4px; } QComboBox QAbstractItemView { color: black; }");  // 添加 color: black;
    genderLayout->addWidget(genderLabel);
    genderLayout->addWidget(genderCombo);

    // 个性签名
    QVBoxLayout *sigLayout = new QVBoxLayout();
    QLabel *sigLabel = new QLabel("个性签名：");
    sigLabel->setStyleSheet("font-size: 14px; color: #333333;");
    QTextEdit *sigEdit = new QTextEdit();
    sigEdit->setText(m_userSignature);
    sigEdit->setMaximumHeight(100);
    sigEdit->setStyleSheet("QTextEdit { color: black; padding: 6px; border: 1px solid #d0d0d0; border-radius: 4px; }");  // 添加 color: black;
    sigLayout->addWidget(sigLabel);
    sigLayout->addWidget(sigEdit);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *saveBtn = new QPushButton("保存");
    QPushButton *cancelBtn = new QPushButton("取消");

    QString btnStyle =
        "QPushButton {"
        "   background-color: rgba(140, 21, 21, 0.08);"
        "   color: #444444;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 5px;"
        "   padding: 8px 25px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(140, 21, 21, 0.15);"
        "   color: #8c1515;"
        "}";

    saveBtn->setStyleSheet(btnStyle);
    cancelBtn->setStyleSheet(btnStyle);

    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);

    // 添加到主布局
    layout->addLayout(idLayout);
    layout->addSpacing(10);
    layout->addLayout(nameLayout);
    layout->addSpacing(10);
    layout->addLayout(ageLayout);
    layout->addSpacing(10);
    layout->addLayout(genderLayout);
    layout->addSpacing(10);
    layout->addLayout(sigLayout);
    layout->addStretch();
    layout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, &dialog, [&]() {
        m_userName = nameEdit->text().trimmed();
        if (m_userName.isEmpty()) m_userName = "未设置";

        bool ok;
        int age = ageEdit->text().trimmed().toInt(&ok);
        if (ok && age > 0 && age < 150) {
            m_userAge = age;
        }

        m_userGender = genderCombo->currentText();
        m_userSignature = sigEdit->toPlainText().trimmed();
        if (m_userSignature.isEmpty()) m_userSignature = "这个人很懒，什么都没写~";

        saveUserProfile();
        updateUserInfoDisplay();
        dialog.accept();
        QMessageBox::information(this, "提示", "个人信息已保存！");
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    dialog.exec();
}

// 圆形头像裁剪函数
QPixmap Widget::getCircularAvatar(const QPixmap& source)
{
    int size = qMin(source.width(), source.height());

    // 先裁剪为正方形
    QPixmap square = source.copy(
        (source.width() - size) / 2,
        (source.height() - size) / 2,
        size, size
        );

    // 缩放到目标尺寸
    int targetSize = 80;  // 头像显示大小
    QPixmap scaled = square.scaled(targetSize, targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 创建圆形遮罩
    QPixmap circular(targetSize, targetSize);
    circular.fill(Qt::transparent);

    QPainter painter(&circular);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(scaled));
    painter.setPen(Qt::NoPen);

    // 绘制圆形
    painter.drawEllipse(0, 0, targetSize, targetSize);
    painter.end();

    return circular;
}

void Widget::onChangeAvatar()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "选择头像",
                                                    "",
                                                    "图片文件 (*.png *.jpg *.jpeg *.bmp)");

    if (!filePath.isEmpty()) {
        QPixmap avatar(filePath);
        if (!avatar.isNull()) {
            // 裁剪为正方形并生成圆形头像
            m_userAvatar = getCircularAvatar(avatar);
            updateUserInfoDisplay();
            saveUserProfile();
            QMessageBox::information(this, "提示", "头像已更换！");
        } else {
            QMessageBox::warning(this, "错误", "无法加载图片，请选择有效的图片文件");
        }
    }
}

void Widget::onLogout()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "确认退出",
                                                              "确定要退出登录吗？",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "提示", "已退出登录");
        // 这里可以添加真正的退出逻辑，比如关闭窗口或返回登录界面
        // this->close();
    }
}
