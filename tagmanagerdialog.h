#ifndef TAGMANAGERDIALOG_H
#define TAGMANAGERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

class TagManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TagManagerDialog(QWidget *parent = nullptr);

    QStringList getExpenseTags() const;
    QStringList getIncomeTags() const;
    void saveTagsToFile();

private slots:
    void onExpenseAdd();
    void onExpenseDelete();
    void onIncomeAdd();
    void onIncomeDelete();

private:
    void setupUI();
    void loadTagsFromFile();
    void initDefaultTags();

    QListWidget *m_expenseList;
    QListWidget *m_incomeList;

    QPushButton *m_btnExpenseAdd;
    QPushButton *m_btnExpenseDelete;

    QPushButton *m_btnIncomeAdd;
    QPushButton *m_btnIncomeDelete;

    QStringList m_expenseTags;
    QStringList m_incomeTags;
};

#endif // TAGMANAGERDIALOG_H