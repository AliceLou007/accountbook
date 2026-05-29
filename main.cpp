#include "widget.h"
#include <QApplication>
#include "addone.h" // 1. 引入我们刚刚写的录入功能头文件

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 2. 临时测试：直接把程序入口改成你的录入界面
    AddOne w;
    w.show();

    return QApplication::exec();
}