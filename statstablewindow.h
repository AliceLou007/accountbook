#ifndef STATSTABLEWINDOW_H
#define STATSTABLEWINDOW_H

#include <QWidget>
#include <QTableWidget>

class statstablewindow : public QWidget
{
    Q_OBJECT
public:
    explicit statstablewindow(QWidget *parent = nullptr);

private:
    QTableWidget *table;
    void loadAndShow();
};

#endif