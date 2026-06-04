#include "manage.h"
#include <QInputDialog>
#include "createbookdialog.h"
#include "editbookdialog.h"
#include "bookdetail.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QDate>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>

Manage::Manage(QWidget *parent)
    : QWidget(parent)
    , m_hoverRow(-1)
    , m_selectedRow(-1)
    , m_currentDetailRow(-1)
    , m_currentBookName("未选择")
{
    setupUI();
    loadBooksFromFile();
    loadSelectedBook();
    loadAccountBooks();
    setupTableHover();
    // 在 Manage 构造函数末尾，替换之前的网络连接代码为：
    NetworkClient* client = NetworkClient::getInstance();
    connect(client, &NetworkClient::connected, this, &Manage::onConnected);
    connect(client, &NetworkClient::disconnected, this, &Manage::onDisconnected);
    connect(client, &NetworkClient::errorOccurred, this, &Manage::onError);
    connect(client, &NetworkClient::createBookResult, this, &Manage::onCreateBookResult);
    connect(client, &NetworkClient::joinBookResult, this, &Manage::onJoinBookResult);
    connect(client, &NetworkClient::getBooksResult, this, &Manage::onGetBooksResult);
    connect(client, &NetworkClient::memberJoined, this, &Manage::onMemberJoined);
    connect(client, &NetworkClient::memberLeft, this, &Manage::onMemberLeft);
    connect(client, &NetworkClient::newRecord, this, &Manage::onNewRecord);
}

Manage::~Manage()
{
    saveSelectedBook();
    saveBooksToFile();
}

void Manage::setupUI()
{
    setWindowTitle("账本管理");
    setFixedSize(700, 900);

    // 设置白色背景
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    palette.setColor(QPalette::WindowText, Qt::black);
    setPalette(palette);
    setAutoFillBackground(true);

    // 创建堆叠窗口
    m_stackedWidget = new QStackedWidget(this);

    // ========== 创建管理页面 ==========
    m_managePage = new QWidget(this);
    QVBoxLayout *manageLayout = new QVBoxLayout(m_managePage);
    manageLayout->setSpacing(10);
    manageLayout->setContentsMargins(20, 20, 20, 20);

    // 标题
    QLabel *titleLabel = new QLabel("账本管理", m_managePage);
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

    // 三个按钮
    m_btnCreate = new QPushButton("📒 创建新账本", m_managePage);
    m_btnInvite = new QPushButton("👥 加入成员进我的账本", m_managePage);
    m_btnTags = new QPushButton("🏷️ 标签分类设置", m_managePage);

    QString btnStyle =
        "QPushButton {"
        "   background-color: rgba(140, 21, 21, 0.08);"
        "   color: #444444;"
        "   border: 1px solid #d0d0d0;"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   font-size: 14px;"
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

    m_btnCreate->setStyleSheet(btnStyle);
    m_btnInvite->setStyleSheet(btnStyle);
    m_btnTags->setStyleSheet(btnStyle);

    QHBoxLayout *topBtnLayout = new QHBoxLayout();
    topBtnLayout->addWidget(m_btnCreate);
    topBtnLayout->addWidget(m_btnInvite);
    topBtnLayout->addWidget(m_btnTags);
    topBtnLayout->setSpacing(20);
    topBtnLayout->setContentsMargins(30, 15, 30, 15);

    // 表格
    m_table = new QTableWidget(m_managePage);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels(QStringList() << "账本名称" << "创建时间" << "成员数" << "记录条数");
    m_table->verticalHeader()->setVisible(false);

    QHeaderView* header = m_table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Fixed);
    header->setSectionResizeMode(2, QHeaderView::Fixed);
    header->setSectionResizeMode(3, QHeaderView::Stretch);

    m_table->setColumnWidth(0, 180);
    m_table->setColumnWidth(1, 120);
    m_table->setColumnWidth(2, 80);

    m_table->setAlternatingRowColors(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setShowGrid(true);
    m_table->setFixedHeight(450);
    m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_table->setStyleSheet(
        "QTableWidget {"
        "   background-color: white;"
        "   gridline-color: #e0e0e0;"
        "   outline: 0px;"
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

    // ========== 当前账本显示区域（可点击切换） ==========
    QFrame *currentBookFrame = new QFrame(m_managePage);
    currentBookFrame->setStyleSheet(
        "QFrame {"
        "   background-color: #fafafa;"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 8px;"
        "}"
        );
    currentBookFrame->setFixedHeight(50);

    QHBoxLayout *currentBookLayout = new QHBoxLayout(currentBookFrame);

    QLabel *currentBookTitle = new QLabel("当前账本：", currentBookFrame);
    currentBookTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333; background-color: transparent;");

    m_currentBookBtn = new QPushButton("未选择", currentBookFrame);
    m_currentBookBtn->setStyleSheet(
        "QPushButton {"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   color: #8c1515;"
        "   background-color: transparent;"
        "   border: none;"
        "   padding: 5px;"
        "}"
        "QPushButton:hover {"
        "   text-decoration: underline;"
        "   background-color: rgba(140, 21, 21, 0.08);"
        "   border-radius: 5px;"
        "}"
        );
    m_currentBookBtn->setCursor(Qt::PointingHandCursor);
    m_currentBookBtn->setMinimumWidth(150);

    QLabel *clickHint = new QLabel("（点击切换）", currentBookFrame);
    clickHint->setStyleSheet("font-size: 11px; color: #999999; background-color: transparent;");

    currentBookLayout->addWidget(currentBookTitle);
    currentBookLayout->addWidget(m_currentBookBtn);
    currentBookLayout->addWidget(clickHint);
    currentBookLayout->addStretch();

    manageLayout->addWidget(titleLabel);
    manageLayout->addSpacing(10);
    manageLayout->addLayout(topBtnLayout);
    manageLayout->addSpacing(10);
    manageLayout->addWidget(currentBookFrame);
    manageLayout->addSpacing(10);
    manageLayout->addWidget(m_table);
    manageLayout->addStretch();
    // 在 manageLayout->addWidget(m_table); 之后添加

    // 添加滚动提示
    QLabel *scrollHint = new QLabel("?? 提示：列表支持滚动，可上下滑动查看更多账本", m_managePage);
    scrollHint->setStyleSheet(
        "QLabel {"
        "   font-size: 11px;"
        "   color: #999999;"
        "   background-color: transparent;"
        "   padding: 5px;"
        "}"
        );
    scrollHint->setAlignment(Qt::AlignCenter);
    manageLayout->addWidget(scrollHint);

    // ========== 创建详情页面 ==========
    m_detailPage = nullptr;

    m_stackedWidget->addWidget(m_managePage);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    // 连接信号
    connect(m_btnCreate, &QPushButton::clicked, this, &Manage::onCreateBook);
    connect(m_btnInvite, &QPushButton::clicked, this, &Manage::onInviteMember);
    connect(m_btnTags, &QPushButton::clicked, this, &Manage::onEditTags);
    connect(m_table, &QTableWidget::itemClicked, this, &Manage::onSwitchBook);
    connect(m_currentBookBtn, &QPushButton::clicked, this, &Manage::onCurrentBookClicked);

    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_table, &QTableWidget::customContextMenuRequested, this, &Manage::onTableContextMenu);
}

void Manage::loadBooksFromFile()
{
    QString filePath = QDir::currentPath() + "/books.json";
    QFile file(filePath);

    if (!file.exists()) {
        // 文件不存在，创建默认账本
        BookInfo defaultBook;
        defaultBook.name = "我的账本";
        defaultBook.createTime = QDate::currentDate().toString("yyyy-MM-dd");
        defaultBook.memberCount = 1;
        defaultBook.recordCount = 0;
        defaultBook.remark = "默认账本";
        defaultBook.isDefault = true;  // 标记为默认账本
        m_books.append(defaultBook);
        saveBooksToFile();

        // 创建对应的数据文件
        QString dataFileName = "我的账本_data.txt";
        if (!QFile::exists(dataFileName)) {
            QFile dataFile(dataFileName);
            dataFile.open(QIODevice::WriteOnly);
            dataFile.close();
        }
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开文件:" << filePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return;
    }

    m_books.clear();
    QJsonArray array = doc.array();
    bool hasDefault = false;

    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        BookInfo book;
        book.name = obj["name"].toString();
        book.createTime = obj["createTime"].toString();
        book.memberCount = obj["memberCount"].toInt();
        book.recordCount = obj["recordCount"].toInt();
        book.remark = obj["remark"].toString();
        book.isDefault = obj["isDefault"].toBool(false);  // 读取是否为默认账本

        if (book.name == "我的账本") {
            book.isDefault = true;
            hasDefault = true;
        }
        m_books.append(book);
    }

    // 如果没有默认账本，添加一个
    if (!hasDefault) {
        BookInfo defaultBook;
        defaultBook.name = "我的账本";
        defaultBook.createTime = QDate::currentDate().toString("yyyy-MM-dd");
        defaultBook.memberCount = 1;
        defaultBook.recordCount = 0;
        defaultBook.remark = "默认账本";
        defaultBook.isDefault = true;
        m_books.prepend(defaultBook);  // 放在最前面

        // 创建对应的数据文件
        QString dataFileName = "我的账本_data.txt";
        if (!QFile::exists(dataFileName)) {
            QFile dataFile(dataFileName);
            dataFile.open(QIODevice::WriteOnly);
            dataFile.close();
        }

        saveBooksToFile();
    }
}

void Manage::saveBooksToFile()
{
    QString filePath = QDir::currentPath() + "/books.json";
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法保存文件:" << filePath;
        return;
    }

    QJsonArray array;
    for (const BookInfo &book : m_books) {
        QJsonObject obj;
        obj["name"] = book.name;
        obj["createTime"] = book.createTime;
        obj["memberCount"] = book.memberCount;
        obj["recordCount"] = book.recordCount;
        obj["remark"] = book.remark;
        obj["isDefault"] = book.isDefault;  // 保存是否为默认账本
        array.append(obj);
    }

    QJsonDocument doc(array);
    file.write(doc.toJson());
    file.close();
}

void Manage::saveSelectedBook()
{
    QString filePath = QDir::currentPath() + "/selected_book.json";
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法保存选中的账本";
        return;
    }

    QJsonObject obj;
    obj["selectedRow"] = m_selectedRow;
    if (m_selectedRow >= 0 && m_selectedRow < m_books.size()) {
        obj["selectedBookName"] = m_books[m_selectedRow].name;
    } else {
        obj["selectedBookName"] = "";
    }

    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
}

void Manage::loadSelectedBook()
{
    QString filePath = QDir::currentPath() + "/selected_book.json";
    QFile file(filePath);

    if (!file.exists()) {
        if (m_books.size() > 0) {
            m_selectedRow = 0;
        } else {
            m_selectedRow = -1;
        }
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法加载选中的账本";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject obj = doc.object();
    QString savedBookName = obj["selectedBookName"].toString();

    // 查找保存的账本是否还存在
    bool found = false;
    for (int i = 0; i < m_books.size(); ++i) {
        if (m_books[i].name == savedBookName) {
            m_selectedRow = i;
            found = true;
            break;
        }
    }

    if (!found) {
        if (m_books.size() > 0) {
            m_selectedRow = 0;
        } else {
            m_selectedRow = -1;
        }
    }

    // 更新当前账本按钮文字
    updateCurrentBookLabel();
}

void Manage::setupTableHover()
{
    m_table->installEventFilter(this);
    m_table->viewport()->installEventFilter(this);
    m_table->setMouseTracking(true);
    m_table->viewport()->setMouseTracking(true);
    m_table->setAttribute(Qt::WA_Hover, true);
    m_table->viewport()->setAttribute(Qt::WA_Hover, true);
}

bool Manage::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_table->viewport() && event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint pos = mouseEvent->pos();
        int row = m_table->rowAt(pos.y());

        if (row != m_hoverRow) {
            // 恢复之前的悬停行
            if (m_hoverRow >= 0 && m_hoverRow < m_table->rowCount()) {
                for (int col = 0; col < m_table->columnCount(); ++col) {
                    QTableWidgetItem *item = m_table->item(m_hoverRow, col);
                    if (item) {
                        item->setBackground(Qt::white);
                        if (m_hoverRow == m_selectedRow && col == 0) {
                            item->setForeground(QColor(140, 21, 21));
                        } else {
                            item->setForeground(Qt::black);
                        }
                    }
                }
            }

            m_hoverRow = row;

            // 设置新的悬停行（包括当前账本行也要变色）
            if (m_hoverRow >= 0 && m_hoverRow < m_table->rowCount()) {
                QColor hoverColor(140, 21, 21, 30);  // 半透明红色
                for (int col = 0; col < m_table->columnCount(); ++col) {
                    QTableWidgetItem *item = m_table->item(m_hoverRow, col);
                    if (item) {
                        item->setBackground(hoverColor);
                        // 悬停时文字保持原来的颜色
                        if (m_hoverRow == m_selectedRow && col == 0) {
                            item->setForeground(QColor(140, 21, 21));
                        } else {
                            item->setForeground(Qt::black);
                        }
                    }
                }
            }
        }
    }
    else if (obj == m_table->viewport() && event->type() == QEvent::Leave) {
        if (m_hoverRow >= 0 && m_hoverRow < m_table->rowCount()) {
            for (int col = 0; col < m_table->columnCount(); ++col) {
                QTableWidgetItem *item = m_table->item(m_hoverRow, col);
                if (item) {
                    item->setBackground(Qt::white);
                    if (m_hoverRow == m_selectedRow && col == 0) {
                        item->setForeground(QColor(140, 21, 21));
                    } else {
                        item->setForeground(Qt::black);
                    }
                }
            }
            m_hoverRow = -1;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void Manage::onCreateBook()
{
    CreateBookDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString bookName = dialog.getBookName();
        QString remark = dialog.getRemark();

        bool exists = false;
        for (const BookInfo &book : m_books) {
            if (book.name == bookName) {
                exists = true;
                break;
            }
        }

        if (exists) {
            QMessageBox::warning(this, "创建失败", "账本名称已存在，请使用其他名称");
            return;
        }

        BookInfo newBook;
        newBook.name = bookName;
        newBook.createTime = QDate::currentDate().toString("yyyy-MM-dd");
        newBook.memberCount = 1;
        newBook.recordCount = 0;  // 新账本记录数为0
        newBook.remark = remark;

        m_books.append(newBook);

        // 创建对应的数据文件（空文件）
        QString dataFileName = QString("%1_data.txt").arg(bookName);
        QFile dataFile(dataFileName);
        if (!dataFile.exists()) {
            dataFile.open(QIODevice::WriteOnly);
            dataFile.close();
        }

        saveBooksToFile();
        loadAccountBooks();  // 这会重新统计并显示

        QMessageBox::information(this, "创建成功", QString("账本 \"%1\" 创建成功！").arg(bookName));
    }
}

void Manage::onInviteMember()
{
    NetworkClient* client = NetworkClient::getInstance();

    // 如果未连接，尝试连接
    if (!client->isConnected()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "连接服务器",
            "未连接到服务器，是否尝试连接？\\\\n请确保服务器程序已运行。",
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            client->connectToServer("127.0.0.1", 8888);

            // 等待2秒看是否连接成功
            QEventLoop loop;
            QTimer::singleShot(2000, &loop, &QEventLoop::quit);
            loop.exec();

            if (!client->isConnected()) {
                QMessageBox::warning(this, "连接失败",
                                     "无法连接到服务器，请检查：\\\\n"
                                     "1. 服务器程序是否已启动\\\\n"
                                     "2. 端口8888是否被占用\\\\n"
                                     "3. 防火墙是否允许连接");
                return;
            }
        } else {
            return;
        }
    }

    CreateBookDialog dialog(this);
    dialog.setWindowTitle("创建多人账本");

    if (dialog.exec() == QDialog::Accepted) {
        QString bookName = dialog.getBookName();
        QString remark = dialog.getRemark();

        client->createBook(bookName, remark);
        QMessageBox::information(this, "提示", "正在创建多人账本，请稍候...");
    }
}

void Manage::onEditTags()
{
    QMessageBox::information(this, "标签分类设置", "点击了标签分类设置按钮\\n实际应用中应弹出标签管理对话框");
}

void Manage::updateCurrentBookLabel()
{
    if (m_selectedRow >= 0 && m_selectedRow < m_books.size()) {
        m_currentBookName = m_books[m_selectedRow].name;
        m_currentBookBtn->setText(m_currentBookName);
    } else {
        m_currentBookName = "未选择";
        m_currentBookBtn->setText("未选择");
    }
}

void Manage::onCurrentBookClicked()
{
    if (m_books.isEmpty()) {
        QMessageBox::warning(this, "提示", "没有可用的账本");
        return;
    }

    QStringList bookNames;
    for (const BookInfo &book : m_books) {
        bookNames << book.name;
    }

    bool ok;
    QString selectedBook = QInputDialog::getItem(this, "切换账本", "请选择要切换的账本：", bookNames, m_selectedRow, false, &ok);

    if (ok && !selectedBook.isEmpty()) {
        for (int i = 0; i < m_books.size(); ++i) {
            if (m_books[i].name == selectedBook) {
                m_selectedRow = i;
                updateCurrentBookLabel();
                loadAccountBooks();
                saveSelectedBook();  // 保存选中的账本到文件
                QMessageBox::information(this, "切换成功", QString("已切换到账本：%1").arg(selectedBook));
                break;
            }
        }
    }
}

void Manage::loadAccountBooks()
{
    // 先更新每个账本的记录条数
    for (int i = 0; i < m_books.size(); ++i) {
        QString dataFileName = QString("%1_data.txt").arg(m_books[i].name);
        QFile dataFile(dataFileName);

        int recordCount = 0;
        if (dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&dataFile);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty()) {
                    recordCount++;
                }
            }
            dataFile.close();
        }
        m_books[i].recordCount = recordCount;
    }

    saveBooksToFile();

    m_table->setRowCount(0);

    for (int i = 0; i < m_books.size(); ++i) {
        m_table->insertRow(i);

        // 当前账本显示红色竖杠
        QString displayName = m_books[i].name;
        if (i == m_selectedRow) {
            displayName = "? " + m_books[i].name;  // 红色竖杠
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(displayName);
        nameItem->setTextAlignment(Qt::AlignCenter);
        nameItem->setBackground(Qt::white);

        if (i == m_selectedRow) {
            nameItem->setForeground(QColor(140, 21, 21));  // 红色文字
            // 设置字体加粗
            QFont font = nameItem->font();
            font.setBold(true);
            nameItem->setFont(font);
        } else {
            nameItem->setForeground(Qt::black);
        }

        QTableWidgetItem *timeItem = new QTableWidgetItem(m_books[i].createTime);
        timeItem->setTextAlignment(Qt::AlignCenter);
        timeItem->setForeground(Qt::black);
        timeItem->setBackground(Qt::white);

        QTableWidgetItem *memberItem = new QTableWidgetItem(QString::number(m_books[i].memberCount) + " 人");
        memberItem->setTextAlignment(Qt::AlignCenter);
        memberItem->setForeground(Qt::black);
        memberItem->setBackground(Qt::white);

        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(m_books[i].recordCount) + " 条");
        countItem->setTextAlignment(Qt::AlignCenter);
        countItem->setForeground(Qt::black);
        countItem->setBackground(Qt::white);

        m_table->setItem(i, 0, nameItem);
        m_table->setItem(i, 1, timeItem);
        m_table->setItem(i, 2, memberItem);
        m_table->setItem(i, 3, countItem);
    }

    m_table->verticalHeader()->setDefaultSectionSize(45);
    updateCurrentBookLabel();
}

void Manage::onSwitchBook(QTableWidgetItem *item)
{
    if (!item) return;

    int row = item->row();
    if (row >= 0 && row < m_books.size()) {
        showBookDetail(row);
    }
}

void Manage::showBookDetail(int row)
{
    if (row < 0 || row >= m_books.size()) return;

    m_currentDetailRow = row;

    if (m_detailPage) {
        m_stackedWidget->removeWidget(m_detailPage);
        delete m_detailPage;
        m_detailPage = nullptr;
    }

    QString bookName = m_books[row].name;
    QString remark = m_books[row].remark;
    QStringList members;
    members << "我";

    m_detailPage = new BookDetail(bookName, remark, members, this);
    connect(m_detailPage, &BookDetail::backToManage, this, &Manage::onBackToManage);

    m_stackedWidget->addWidget(m_detailPage);
    m_stackedWidget->setCurrentWidget(m_detailPage);
}

void Manage::onBackToManage()
{
    m_stackedWidget->setCurrentWidget(m_managePage);
    loadAccountBooks();
}

void Manage::onEditBook()
{
    if (m_currentRightClickRow < 0 || m_currentRightClickRow >= m_books.size()) {
        return;
    }

    BookInfo &book = m_books[m_currentRightClickRow];

    // 检查是否为默认账本
    if (book.isDefault || book.name == "我的账本") {
        QMessageBox::warning(this, "提示", "默认账本\"我的账本\"不能修改名称");
        return;
    }

    QString oldName = book.name;

    EditBookDialog dialog(this);
    dialog.setBookInfo(book.name, book.remark);

    if (dialog.exec() == QDialog::Accepted) {
        QString newName = dialog.getBookName();
        QString newRemark = dialog.getRemark();

        bool exists = false;
        for (int i = 0; i < m_books.size(); ++i) {
            if (i != m_currentRightClickRow && m_books[i].name == newName) {
                exists = true;
                break;
            }
        }

        if (exists) {
            QMessageBox::warning(this, "修改失败", "账本名称已存在，请使用其他名称");
            return;
        }

        // 重命名数据文件
        if (oldName != newName) {
            QString oldDataFileName = QString("%1_data.txt").arg(oldName);
            QString newDataFileName = QString("%1_data.txt").arg(newName);
            QFile::rename(oldDataFileName, newDataFileName);
        }

        book.name = newName;
        book.remark = newRemark;

        saveBooksToFile();

        if (m_currentRightClickRow == m_selectedRow) {
            saveSelectedBook();
        }

        loadAccountBooks();

        QMessageBox::information(this, "修改成功", QString("账本 \"%1\" 修改成功！").arg(newName));
    }
}

void Manage::onDeleteBook()
{
    if (m_currentRightClickRow < 0 || m_currentRightClickRow >= m_books.size()) {
        return;
    }

    QString bookName = m_books[m_currentRightClickRow].name;
    bool isDefault = m_books[m_currentRightClickRow].isDefault;

    // 检查是否为默认账本
    if (isDefault || bookName == "我的账本") {
        QMessageBox::warning(this, "提示", "默认账本\"我的账本\"不能删除");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除账本 \"%1\" 吗？\\\\n删除后无法恢复！\\\\n该账本的所有记录也会被删除！").arg(bookName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        // 删除对应的数据文件
        QString dataFileName = QString("%1_data.txt").arg(bookName);
        QFile::remove(dataFileName);

        bool isDeletingSelected = (m_currentRightClickRow == m_selectedRow);

        m_books.removeAt(m_currentRightClickRow);

        if (isDeletingSelected) {
            if (m_books.size() > 0) {
                // 优先选择默认账本
                for (int i = 0; i < m_books.size(); ++i) {
                    if (m_books[i].isDefault || m_books[i].name == "我的账本") {
                        m_selectedRow = i;
                        break;
                    }
                }
                if (m_selectedRow == -1) {
                    m_selectedRow = 0;
                }
            } else {
                m_selectedRow = -1;
            }
        } else if (m_currentRightClickRow < m_selectedRow) {
            m_selectedRow--;
        }

        saveSelectedBook();
        saveBooksToFile();
        loadAccountBooks();
        QMessageBox::information(this, "删除成功", "账本已删除");
    }
}

void Manage::onTableContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = m_table->itemAt(pos);
    if (item) {
        m_currentRightClickRow = item->row();

        // 检查是否为默认账本
        bool isDefault = m_books[m_currentRightClickRow].isDefault ||
                         m_books[m_currentRightClickRow].name == "我的账本";

        QMenu menu(this);

        if (!isDefault) {
            QAction *editAction = menu.addAction("修改账本");
            QAction *deleteAction = menu.addAction("删除账本");
            connect(editAction, &QAction::triggered, this, &Manage::onEditBook);
            connect(deleteAction, &QAction::triggered, this, &Manage::onDeleteBook);
            menu.addSeparator();
        }

        QAction *joinAction = menu.addAction("加入多人账本");
        connect(joinAction, &QAction::triggered, this, &Manage::showJoinBookDialog);

        menu.exec(m_table->viewport()->mapToGlobal(pos));
    } else {
        // 在空白区域右键
        QMenu menu(this);
        QAction *joinAction = menu.addAction("加入多人账本");
        connect(joinAction, &QAction::triggered, this, &Manage::showJoinBookDialog);
        menu.exec(m_table->viewport()->mapToGlobal(pos));
    }
}
// ========== 网络功能实现（只添加，不修改原有代码）==========

void Manage::onConnected()
{
    qDebug() << "已连接到服务器";
    NetworkClient::getInstance()->getBooks();
}

void Manage::onDisconnected()
{
    qDebug() << "已断开服务器连接";
}

void Manage::onError(const QString &error)
{
    QMessageBox::warning(this, "网络错误", error);
}

void Manage::onCreateBookResult(bool success, const QString &bookId, const QString &bookName, const QString &inviteCode, const QString &message)
{
    if (success) {
        QMessageBox::information(this, "创建成功",
                                 QString("多人账本 \"%1\" 创建成功！\\\\n邀请码：%2\\\\n请保存好邀请码以便他人加入")
                                     .arg(bookName).arg(inviteCode));
        NetworkClient::getInstance()->getBooks();
    } else {
        QMessageBox::warning(this, "创建失败", message);
    }
}

void Manage::onJoinBookResult(bool success, const QString &bookName, const QString &message)
{
    if (success) {
        QMessageBox::information(this, "加入成功", QString("成功加入账本：%1").arg(bookName));
        NetworkClient::getInstance()->getBooks();
    } else {
        QMessageBox::warning(this, "加入失败", message);
    }
}

void Manage::onGetBooksResult(const QJsonArray &books)
{
    // 将服务器账本添加到本地列表（保留原有本地账本）
    for (const QJsonValue &value : books) {
        QJsonObject obj = value.toObject();

        // 检查是否已存在
        bool exists = false;
        for (const BookInfo &book : m_books) {
            if (book.bookId == obj["bookId"].toString()) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            BookInfo newBook;
            newBook.name = obj["bookName"].toString();
            newBook.bookId = obj["bookId"].toString();
            newBook.inviteCode = obj["inviteCode"].toString();
            newBook.createTime = obj["createTime"].toString();
            newBook.memberCount = obj["memberCount"].toInt();
            newBook.recordCount = 0;
            newBook.remark = obj["remark"].toString();
            m_books.append(newBook);
        }
    }

    saveBooksToFile();
    loadAccountBooks();
}

void Manage::onMemberJoined(const QString &bookId, const QString &userId, const QString &userName)
{
    Q_UNUSED(bookId);
    Q_UNUSED(userId);
    qDebug() << QString("%1 加入了账本").arg(userName);
    NetworkClient::getInstance()->getBooks();
}

void Manage::onMemberLeft(const QString &bookId, const QString &userId, const QString &userName)
{
    Q_UNUSED(bookId);
    Q_UNUSED(userId);
    qDebug() << QString("%1 离开了账本").arg(userName);
    NetworkClient::getInstance()->getBooks();
}

void Manage::onNewRecord(const QString &bookId, const QJsonObject &record)
{
    Q_UNUSED(bookId);
    Q_UNUSED(record);
    qDebug() << "账本有新记录";
}

void Manage::showJoinBookDialog()
{
    bool ok;
    QString bookId = QInputDialog::getText(this, "加入多人账本",
                                           "请输入账本ID：",
                                           QLineEdit::Normal,
                                           "", &ok);
    if (!ok || bookId.isEmpty()) return;

    QString inviteCode = QInputDialog::getText(this, "加入多人账本",
                                               "请输入邀请码：",
                                               QLineEdit::Normal,
                                               "", &ok);
    if (!ok || inviteCode.isEmpty()) return;

    NetworkClient* client = NetworkClient::getInstance();
    if (client->isConnected()) {
        client->joinBook(bookId, inviteCode);
    } else {
        QMessageBox::warning(this, "错误", "未连接到服务器，请先启动服务器");
    }
}
#include <QShowEvent>  // 文件顶部添加

void Manage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 每次显示窗口时，重新加载选中的账本
    loadSelectedBook();
    loadAccountBooks();  // 刷新表格，高亮正确的账本
}