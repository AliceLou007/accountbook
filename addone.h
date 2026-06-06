#ifndef ADDONE_H
#define ADDONE_H

#include <QDialog>
#include <QStringList>
#include <QLabel>

namespace Ui {
class AddOne;
}

class AddOne : public QDialog
{
    Q_OBJECT

public:
    explicit AddOne(QWidget *parent = nullptr);
    ~AddOne();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void onTypeChanged(const QString &type);
    void onSelectImage();  // 选择图片

private:
    void loadBookNames();
    void loadSelectedBook();
    void updateBookRecordCount(const QString &bookName);
    void loadCategoriesFromTags();
    QStringList getTopThreeRemarks();
    QString copyImageToBook(const QString &imagePath, const QString &bookName);

private:
    Ui::AddOne *ui;
    QStringList m_expenseTags;
    QStringList m_incomeTags;
    QString m_currentImagePath;  // 当前选择的图片路径
    QLabel *m_imageLabel;        // 显示图片状态
};

#endif // ADDONE_H