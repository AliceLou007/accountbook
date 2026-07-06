#include "datachartwidget.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include "userdata.h"

datachartwidget::datachartwidget(QWidget *parent)
    : QWidget(parent)
    , m_bookName("")
{
    // 创建基础布局：上层放两个饼图，下层放折线图
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *pieLayout = new QHBoxLayout();

    m_incomePieView = new QChartView(this);
    m_outcomePieView = new QChartView(this);
    m_trendLineView = new QChartView(this);

    // 开启抗锯齿，让线条和扇形边缘更平滑
    m_incomePieView->setRenderHint(QPainter::Antialiasing);
    m_outcomePieView->setRenderHint(QPainter::Antialiasing);
    m_trendLineView->setRenderHint(QPainter::Antialiasing);

    // 组装布局
    pieLayout->addWidget(m_incomePieView);
    pieLayout->addWidget(m_outcomePieView);
    mainLayout->addLayout(pieLayout, 4);     // 饼图占上层 40% 空间
    mainLayout->addWidget(m_trendLineView, 6); // 折线图占下层 60% 空间

    this->setLayout(mainLayout);
}

datachartwidget::~datachartwidget()
{
}

void datachartwidget::setbookname(const QString &name)
{
    if (!name.isEmpty()) {
        m_bookName = name;
    } else {
        m_bookName = getCurrentBook(); // 如果传空，则动态读取当前账本
    }
}

QString datachartwidget::getCurrentBook()
{
    QFile f(UserData::selectedBookFile());
    if (f.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        return doc.object()["selectedBookName"].toString("我的账本");
    }
    return "我的账本";
}

void datachartwidget::loaddata()
{
    // 确保账本名是最新的
    if (m_bookName.isEmpty()) {
        m_bookName = getCurrentBook();
    }

    QFile file(UserData::recordFile(m_bookName));

    // 准备存储结构
    QString currentMonth = QDate::currentDate().toString("yyyy-MM");
    QMap<QString, double> incomeCategories; // 本月收入分类 -> 金额
    QMap<QString, double> outcomeCategories;// 本月支出分类 -> 金额
    QMap<QString, QPair<double, double>> trendData; // 所有历史月份 -> QPair<总收入, 总支出>

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) continue;

            QStringList tokens = line.split(",");
            if (tokens.size() >= 6) {
                QString qDate = tokens[1];
                QString ym = qDate.left(7).replace("/", "-"); // 转换为 "yyyy-MM"
                QString qType = tokens[2];
                QString qCategory = tokens[3];
                double amount = tokens[4].toDouble();

                // 1. 筛选本月数据，用于饼图
                if (ym == currentMonth) {
                    if (qType == "收入") {
                        incomeCategories[qCategory] += amount;
                    } else if (qType == "支出") {
                        outcomeCategories[qCategory] += amount;
                    }
                }

                // 2. 统计所有月份数据，用于折线图
                if (qType == "收入") {
                    trendData[ym].first += amount;
                } else if (qType == "支出") {
                    trendData[ym].second += amount;
                }
            }
        }
        file.close();
    }

    // ==================== 国风专属色盘 ====================
    QList<QColor> classicColors = {
        QColor("#9E2A2B"), // 朱砂红 (配合你的主色调)
        QColor("#3A5F43"), // 翡翠绿
        QColor("#D4A373"), // 琥珀黄
        QColor("#2B4C7E"), // 黛蓝
        QColor("#6B5B95"), // 暮紫
        QColor("#86A3C3")  // 浅缥
    };

    // ==================== 1. 渲染本月收入饼图 ====================
    QChart *incomeChart = new QChart();
    incomeChart->setTitle(QString("本月收入分类占比 (%1)").arg(currentMonth));
    QPieSeries *incomeSeries = new QPieSeries();

    int colorIdx = 0;
    for (auto it = incomeCategories.begin(); it != incomeCategories.end(); ++it) {
        QPieSlice *slice = incomeSeries->append(it.key(), it.value());
        slice->setLabel(QString("%1: ￥%2").arg(it.key()).arg(it.value(), 0, 'f', 2));
        slice->setBrush(classicColors[colorIdx % classicColors.size()]);
        colorIdx++;
    }
    incomeChart->addSeries(incomeSeries);
    incomeChart->legend()->setAlignment(Qt::AlignBottom);
    m_incomePieView->setChart(incomeChart);

    // ==================== 2. 渲染本月支出饼图 ====================
    QChart *outcomeChart = new QChart();
    outcomeChart->setTitle(QString("本月支出分类占比 (%1)").arg(currentMonth));
    QPieSeries *outcomeSeries = new QPieSeries();

    colorIdx = 0; // 重置颜色索引
    for (auto it = outcomeCategories.begin(); it != outcomeCategories.end(); ++it) {
        QPieSlice *slice = outcomeSeries->append(it.key(), it.value());
        slice->setLabel(QString("%1: ￥%2").arg(it.key()).arg(it.value(), 0, 'f', 2));
        slice->setBrush(classicColors[colorIdx % classicColors.size()]);
        colorIdx++;
    }
    outcomeChart->addSeries(outcomeSeries);
    outcomeChart->legend()->setAlignment(Qt::AlignBottom);
    m_outcomePieView->setChart(outcomeChart);

    // ==================== 3. 渲染历史收支趋势折线图 ====================
    QChart *lineChart = new QChart();
    lineChart->setTitle("过往历史月度收支趋势");

    QLineSeries *incomeLine = new QLineSeries();
    incomeLine->setName("月度总收入");
    incomeLine->setColor(QColor("#3A5F43")); // 翡翠绿线

    QLineSeries *outcomeLine = new QLineSeries();
    outcomeLine->setName("月度总支出");
    outcomeLine->setColor(QColor("#9E2A2B")); // 朱砂红线

    // 横轴坐标分类（利用 QMap 自动按月份字符串升序排序的特性）
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    double maxAmount = 100.0; // 初始Y轴最大范围上限
    int monthIndex = 0;

    for (auto it = trendData.begin(); it != trendData.end(); ++it) {
        axisX->append(it.key()); // 添加诸如 "2026-04" 的月份标签

        incomeLine->append(monthIndex, it.value().first);
        outcomeLine->append(monthIndex, it.value().second);

        // 动态抓取全局最高金额以适配合理的 Y 轴动态刻度
        maxAmount = qMax(maxAmount, qMax(it.value().first, it.value().second));
        monthIndex++;
    }

    lineChart->addSeries(incomeLine);
    lineChart->addSeries(outcomeLine);

    // 配置 X 轴
    lineChart->addAxis(axisX, Qt::AlignBottom);
    incomeLine->attachAxis(axisX);
    outcomeLine->attachAxis(axisX);

    // 配置 Y 轴
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxAmount * 1.15); // 留出 15% 的顶部视觉裕量
    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("金额 (元)");
    lineChart->addAxis(axisY, Qt::AlignLeft);
    incomeLine->attachAxis(axisY);
    outcomeLine->attachAxis(axisY);

    lineChart->legend()->setAlignment(Qt::AlignTop);
    m_trendLineView->setChart(lineChart);
}
