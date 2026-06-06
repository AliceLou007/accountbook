#include "datachartwidget.h"
#include <QtWidgets>
#include <QChartView>
#include <QtCharts>

datachartwidget::datachartwidget(QWidget *parent)
    : QWidget(parent)
{
    setupui();
}

void datachartwidget::setupui()
{
    // 1. 创建整体垂直布局
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(5, 5, 5, 5);

    // 2. 初始化分页主控件
    m_tabwidget = new QTabWidget(this);

    // 3. 【第一页】创建表格，并用明细表 Emoji 装饰标签页
    m_tablewidget = new QTableWidget(this);
    m_tablewidget->setAlternatingRowColors(true);
    m_tablewidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabwidget->addTab(m_tablewidget, "📋 数据明细");

    // 4. 【第二页】创建柱状图画板，塞入第二个分页
    m_barchartview = new QChartView(this);
    m_tabwidget->addTab(m_barchartview, "📊 收支对比图");

    // 5. 【第三页】创建饼图画板，塞入第三个分页
    m_piechartview = new QChartView(this);
    m_tabwidget->addTab(m_piechartview, "🪙 支出占比图");

    // 6. 把大分页控件安放到主布局中
    mainlayout->addWidget(m_tabwidget);
}

void datachartwidget::setbookname(const QString &name)
{
    m_bookname = name;
    loaddata();
}

void datachartwidget::loaddata()
{
    parsedatafile();
    refreshtable();
    refreshbarchart(); // 🌟 运行时，图表会自动画到上面 setupui 准备好的分页画板里！
    refreshpiechart();
}

void datachartwidget::parsedatafile()
{
    m_monthlydata.clear();
    m_categoryexpense.clear();

    if (m_bookname.isEmpty()) {
        QString jsonpath = QDir::currentPath() + "/selected_book.json";
        QFile jsonfile(jsonpath);
        if (jsonfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray jsondata = jsonfile.readAll();
            jsonfile.close();
            QJsonDocument jsondoc = QJsonDocument::fromJson(jsondata);
            if (!jsondoc.isNull() && jsondoc.isObject()) {
                QJsonObject jsonobj = jsondoc.object();
                if (jsonobj.contains("selectedBookName")) {
                    m_bookname = jsonobj.value("selectedBookName").toString();
                }
            }
        }
    }

    if (m_bookname.isEmpty()) {
        m_bookname = "我的账本";
    }

    QString filepath = getdatafilepath();
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", QString("无法打开数据文件：%1").arg(filepath));
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList tokens = line.split(",");
        if (tokens.size() < 6) continue;

        QString qdate     = tokens[1];
        QString qtype     = tokens[2];
        QString qcategory = tokens[3];
        double amount     = tokens[4].toDouble();

        QString yearmonthkey = qdate.left(7);
        yearmonthkey.replace("/", "-");

        if (qtype == "收入") {
            m_monthlydata[yearmonthkey].income += amount;
        } else if (qtype == "支出") {
            m_monthlydata[yearmonthkey].outcome += amount;
            QString category = qcategory.trimmed();
            if (category.isEmpty()) category = "其他";
            m_categoryexpense[category] += amount;
        }
        m_monthlydata[yearmonthkey].count++;
    }
    file.close();

    for (auto it = m_monthlydata.begin(); it != m_monthlydata.end(); ++it) {
        it->balance = it->income - it->outcome;
    }
}

QString datachartwidget::getdatafilepath() const
{
    return QDir::currentPath() + "/" + m_bookname + "_data.txt";
}

void datachartwidget::refreshtable()
{
    QStringList headers = {"月份", "总收入 (元)", "总支出 (元)", "结余 (元)"};
    m_tablewidget->setColumnCount(headers.size());
    m_tablewidget->setHorizontalHeaderLabels(headers);

    QList<QString> months = m_monthlydata.keys();
    std::sort(months.begin(), months.end());
    m_tablewidget->setRowCount(months.size());

    for (int i = 0; i < months.size(); ++i) {
        const QString &month = months[i];
        const monthdata &data = m_monthlydata[month];

        QTableWidgetItem *itemmonth = new QTableWidgetItem(month);
        QTableWidgetItem *itemincome = new QTableWidgetItem(QString::number(data.income, 'f', 2));
        QTableWidgetItem *itemoutcome = new QTableWidgetItem(QString::number(data.outcome, 'f', 2));
        QTableWidgetItem *itembalance = new QTableWidgetItem(QString::number(data.balance, 'f', 2));

        if (data.balance < 0) itembalance->setForeground(Qt::red);
        else if (data.balance > 0) itembalance->setForeground(Qt::darkGreen);

        m_tablewidget->setItem(i, 0, itemmonth);
        m_tablewidget->setItem(i, 1, itemincome);
        m_tablewidget->setItem(i, 2, itemoutcome);
        m_tablewidget->setItem(i, 3, itembalance);
    }
    m_tablewidget->resizeColumnsToContents();
}

void datachartwidget::refreshbarchart()
{
    if (m_monthlydata.isEmpty()) {
        m_barchartview->setChart(new QChart());
        return;
    }

    QList<QString> months = m_monthlydata.keys();
    std::sort(months.begin(), months.end());

    QBarSet *incomeset = new QBarSet("收入");
    QBarSet *outcomeset = new QBarSet("支出");
    for (const QString &month : months) {
        *incomeset << m_monthlydata[month].income;
        *outcomeset << m_monthlydata[month].outcome;
    }

    QBarSeries *series = new QBarSeries();
    series->append(incomeset);
    series->append(outcomeset);
    series->setLabelsVisible(true);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("每月收支对比");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisx = new QBarCategoryAxis();
    axisx->append(months);
    chart->addAxis(axisx, Qt::AlignBottom);
    series->attachAxis(axisx);

    QValueAxis *axisy = new QValueAxis();
    axisy->setTitleText("金额 (元)");
    chart->addAxis(axisy, Qt::AlignLeft);
    series->attachAxis(axisy);

    m_barchartview->setChart(chart);
    m_barchartview->setRenderHint(QPainter::Antialiasing);
}

void datachartwidget::refreshpiechart()
{
    if (m_categoryexpense.isEmpty()) {
        m_piechartview->setChart(new QChart());
        return;
    }

    QPieSeries *series = new QPieSeries();
    for (auto it = m_categoryexpense.begin(); it != m_categoryexpense.end(); ++it) {
        series->append(it.key(), it.value());
    }
    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);
    for (QPieSlice *slice : series->slices()) {
        slice->setLabel(QString("%1\n%2元")
                            .arg(slice->label())
                            .arg(slice->value(), 0, 'f', 2));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("支出类别占比");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignRight);

    m_piechartview->setChart(chart);
    m_piechartview->setRenderHint(QPainter::Antialiasing);
}