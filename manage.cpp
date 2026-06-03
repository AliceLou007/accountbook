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
        BookInfo defaultBook;
        defaultBook.name = "我的账本";
        defaultBook.createTime = QDate::currentDate().toString("yyyy-MM-dd");
        defaultBook.memberCount = 1;
        defaultBook.recordCount = 0;
        defaultBook.remark = "默认账本";
        m_books.append(defaultBook);
        saveBooksToFile();
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
    for (const QJsonValue &value : array) {
        QJsonObject obj = value.toObject();
        BookInfo book;
        book.name = obj["name"].toString();
        book.createTime = obj["createTime"].toString();
        book.memberCount = obj["memberCount"].toInt();
        book.recordCount = obj["recordCount"].toInt();
        book.remark = obj["remark"].toString();
        m_books.append(book);
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

            if (m_hoverRow >= 0 && m_hoverRow < m_table->rowCount() && m_hoverRow != m_selectedRow) {
                QColor hoverColor(140, 21, 21, 30);
                for (int col = 0; col < m_table->columnCount(); ++col) {
                    QTableWidgetItem *item = m_table->item(m_hoverRow, col);
                    if (item) {
                        item->setBackground(hoverColor);
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
        newBook.recordCount = 0;
        newBook.remark = remark;

        m_books.append(newBook);
        saveBooksToFile();
        loadAccountBooks();

        QMessageBox::information(this, "创建成功", QString("账本 \"%1\" 创建成功！").arg(bookName));
    }
}

void Manage::onInviteMember()
{
    int currentRow = m_table->currentRow();
    if (currentRow >= 0) {
        QString bookName = m_table->item(currentRow, 0)->text();
        QMessageBox::information(this, "加入成员", QString("邀请成员加入账本：%1").arg(bookName));
    } else {
        QMessageBox::warning(this, "提示", "请先选择一个账本");
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
                saveSelectedBook();
                QMessageBox::information(this, "切换成功", QString("已切换到账本：%1").arg(selectedBook));
                break;
            }
        }
    }
}

void Manage::loadAccountBooks()
{
    m_table->setRowCount(0);

    for (int i = 0; i < m_books.size(); ++i) {
        m_table->insertRow(i);

        QString displayName = m_books[i].name;
        if (i == m_selectedRow) {
            displayName = "❙ " + m_books[i].name;
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(displayName);
        nameItem->setTextAlignment(Qt::AlignCenter);
        nameItem->setBackground(Qt::white);

        if (i == m_selectedRow) {
            nameItem->setForeground(QColor(140, 21, 21));
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

        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(m_books[i].recordCount));
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

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认删除",
        QString("确定要删除账本 \"%1\" 吗？\n删除后无法恢复！\nps:这个删除部分还需要以后进行再调整").arg(bookName),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        bool isDeletingSelected = (m_currentRightClickRow == m_selectedRow);

        m_books.removeAt(m_currentRightClickRow);

        if (isDeletingSelected) {
            if (m_books.size() > 0) {
                m_selectedRow = 0;
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

        QMenu menu(this);
        QAction *editAction = menu.addAction("✏️ 修改账本");
        QAction *deleteAction = menu.addAction("🗑️ 删除账本");
        menu.addSeparator();

        connect(editAction, &QAction::triggered, this, &Manage::onEditBook);
        connect(deleteAction, &QAction::triggered, this, &Manage::onDeleteBook);

        menu.exec(m_table->viewport()->mapToGlobal(pos));
    }
}