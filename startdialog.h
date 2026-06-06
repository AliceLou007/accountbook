#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>

class QPushButton;

class startdialog : public QDialog
{
    Q_OBJECT
public:
    explicit startdialog(QWidget *parent = nullptr);

private slots:
    void on_showchart_clicked();

private:
    QPushButton *m_btn;
};
#endif