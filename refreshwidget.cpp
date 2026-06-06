#include "datachartwidget.h"
#include <QtWidgets>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

datachartwidget::datachartwidget(QWidget *parent)
    : QWidget(parent)
{
    setupui();
}

void datachartwidget::setupui()
{
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(5, 5, 5, 5);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    m_linechartview = new QChartView(this);
    m_piechartview = new QChartView(this);
    splitter->addWidget(m_linechartview);
    splitter->addWidget(m_piechartview);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    mainlayout->addWidget(splitter);
}

void datachartwidget::setbookname(const QString &name)
{
    m_bookname = name;
    loaddata();
}

void datachartwidget::loaddata()
{
    parsedatafile();
    refreshlinechart();
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

void datachartwidget::refreshlinechart()
{
    if (m_monthlydata.isEmpty()) {
        m_linechartview->setChart(new QChart());
        return;
    }

    // 按月份排序
    QList<QString> months = m_monthlydata.keys();
    std::sort(months.begin(), months.end());

    // 创建支出折线系列
    QLineSeries *expenseSeries = new QLineSeries();
    expenseSeries->setName("支出");
    // 可选：同时显示收入折线（注释掉）
    // QLineSeries *incomeSeries = new QLineSeries();
    // incomeSeries->setName("收入");

    for (int i = 0; i < months.size(); ++i) {
        const QString &month = months[i];
        double expense = m_monthlydata[month].outcome;
        expenseSeries->append(i, expense);  // X轴用索引，下面会设置自定义标签
        // incomeSeries->append(i, m_monthlydata[month].income);
    }

    QChart *chart = new QChart();
    chart->addSeries(expenseSeries);
    // chart->addSeries(incomeSeries);
    chart->setTitle("月度支出趋势");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // X轴：显示月份字符串
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(months);
    chart->addAxis(axisX, Qt::AlignBottom);
    expenseSeries->attachAxis(axisX);
    // incomeSeries->attachAxis(axisX);

    // Y轴：金额
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("金额 (元)");
    chart->addAxis(axisY, Qt::AlignLeft);
    expenseSeries->attachAxis(axisY);
    // incomeSeries->attachAxis(axisY);

    // 启用数据点标签
    expenseSeries->setPointsVisible(true);
    // incomeSeries->setPointsVisible(true);

    m_linechartview->setChart(chart);
    m_linechartview->setRenderHint(QPainter::Antialiasing);
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

    // 设置饼图样式：显示数值（保留两位小数）和标签
    series->setLabelsVisible(true);
    series->setLabelsPosition(QPieSlice::LabelOutside);
    for (QPieSlice *slice : series->slices()) {
        // 显示格式：类别名 + 金额，例如 "饮食\n72.08元"
        slice->setLabel(QString("%1\n%2元")
                            .arg(slice->label())
                            .arg(slice->value(), 0, 'f', 2));
    }

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("支出用途分布");   // 与图片标题一致
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignRight);

    m_piechartview->setChart(chart);
    m_piechartview->setRenderHint(QPainter::Antialiasing);
}