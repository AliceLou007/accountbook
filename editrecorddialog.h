#ifndef EDITRECORDDIALOG_H
#define EDITRECORDDIALOG_H

#include <QDialog>
#include <QDateEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileDialog>
#include <QPixmap>
#include <QDateTime>
#include <QFileInfo>

class EditRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRecordDialog(const QString &date, const QString &type,
                              const QString &category, double amount,
                              const QString &remark, const QString &imagePath = "",
                              QWidget *parent = nullptr);

    QString getDate() const;
    QString getType() const;
    QString getCategory() const;
    double getAmount() const;
    QString getRemark() const;
    QString getImagePath() const;  // 获取图片路径
    bool isImageChanged() const;   // 图片是否改变

private slots:
    void onTypeChanged(const QString &type);
    void onSelectImage();  // 选择图片
    void onViewImage();    // 查看当前图片
    void onOkClicked();
    void onCancelClicked();

private:
    void loadCategoriesFromTags();
    void setupUI();
    QString copyImageToBook(const QString &imagePath, const QString &bookName);

    QDateEdit *m_dateEdit;
    QComboBox *m_typeCombo;
    QComboBox *m_categoryCombo;
    QDoubleSpinBox *m_amountSpin;
    QLineEdit *m_remarkEdit;

    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_selectImageBtn;
    QPushButton *m_viewImageBtn;
    QLabel *m_imageLabel;

    QStringList m_expenseTags;
    QStringList m_incomeTags;

    QString m_originalDate;
    QString m_originalType;
    QString m_originalCategory;
    double m_originalAmount;
    QString m_originalRemark;
    QString m_originalImagePath;
    QString m_currentImagePath;  // 新选择的图片路径
    bool m_imageChanged;
};

#endif // EDITRECORDDIALOG_H