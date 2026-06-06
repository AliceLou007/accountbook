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

    // 水平分割器：左边柱状图，右边饼状图
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    m_barchartview = new QChartView(this);
    m_piechartview = new QChartView(this);
    splitter->addWidget(m_barchartview);
    splitter->addWidget(m_piechartview);
    // 宽度比例 1:1
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
    refreshbarchart();
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