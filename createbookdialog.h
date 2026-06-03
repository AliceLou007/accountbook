#ifndef CREATEBOOKDIALOG_H
#define CREATEBOOKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

class CreateBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateBookDialog(QWidget *parent = nullptr);

    // 获取用户输入
    QString getBookName() const;
    QString getRemark() const;

private slots:
    void onConfirm();
    void onCancel();
    void validateInput();

private:
    void setupUI();

    QLineEdit *m_nameEdit;      // 账本名称输入框
    QTextEdit *m_remarkEdit;    // 备注输入框
    QPushButton *m_btnConfirm;  // 确认按钮
    QPushButton *m_btnCancel;   // 取消按钮
    QLabel *m_errorLabel;       // 错误提示标签
};

#endif // CREATEBOOKDIALOG_H