#ifndef RECORD_H
#define RECORD_H

#include <QWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStringList>
#include <QShowEvent>

struct AccountItem {
    QString book;
    QString date;
    QString type;
    QString category;
    double amount;
    QString remark;
    QString imagePath;  // 添加图片路径字段
};

namespace Ui {
class Record;
}

class Record : public QWidget
{
    Q_OBJECT
private:
    QString copyImageToBookForRecord(const QString &imagePath, const QString &bookName);
public:
    explicit Record(QWidget *parent = nullptr);
    ~Record();

    void loadDataFromFile();
    void updateCurrentBookDisplay();

signals:
    void dataChanged();
    void bookChanged();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void on_currentBookBtn_clicked();
    void on_sortTime_clicked();
    void on_editRecord_clicked(int index);
    void on_deleteRecord_clicked(int index);
    void on_viewImage_clicked(int index);  // 添加查看图片的槽函数

private:
    void loadBookNames();
    void loadSelectedBook();
    void saveSelectedBook();
    void saveDataToFile();
    void displayRecords();
    void filterAndDisplayRecords(const QString &selectedCategory);
    void updateTopbarStyle(QPushButton* activeBtn);
    void loadTagsFromFile();
    void checkAndFixCategories();
    void updateCategoryMenu();

    Ui::Record *ui;
    QList<AccountItem> m_allRecords;
    QStringList m_bookNames;
    QString m_currentBookName;
    int m_selectedRow;
    int m_currentSortType;

    QPushButton *m_currentBookBtn;
};

#endif // RECORD_H