#ifndef EDITBOOKDIALOG_H
#define EDITBOOKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

class EditBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditBookDialog(QWidget *parent = nullptr);

    void setBookInfo(const QString& name, const QString& remark);
    QString getBookName() const;
    QString getRemark() const;

private slots:
    void onConfirm();
    void onCancel();
    void validateInput();

private:
    void setupUI();

    QLineEdit *m_nameEdit;
    QTextEdit *m_remarkEdit;
    QPushButton *m_btnConfirm;
    QPushButton *m_btnCancel;
    QLabel *m_errorLabel;
    QString m_oldName;
};

#endif // EDITBOOKDIALOG_H