#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include "datachartwidget.h"
#include "record.h"
#include "manage.h"
#include "budget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

// MonthStat 结构体定义
struct MonthStat {
    double income = 0.0;      // 总收入
    double outcome = 0.0;     // 总支出
    double balance = 0.0;     // 结余
    int count = 0;            // 记录条数

    MonthStat() {}
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_btnHome_clicked();
    void on_btnHistory_clicked();
    void on_btnCount_clicked();
    void on_btnBudget_clicked();
    void on_btnManagement_clicked();
    void onAvatarClicked();  // 头像点击事件
    void onEditProfile();    // 编辑个人信息
    void onChangeAvatar();   // 更换头像
    void onLogout();         // 退出登录
protected:
    void showEvent(QShowEvent *event) override;

private:
    QPixmap getCircularAvatar(const QPixmap& source);  // 圆形裁剪函数
    void updateHomeUi(const QString &yearMonth);
    void loadAndCalculateAllData();
    void updateSidebarStyle(QPushButton* activeBtn);
    void loadUserProfile();   // 加载用户信息
    void saveUserProfile();   // 保存用户信息
    void updateAvatarDisplay(); // 更新头像显示
    void updateUserInfoDisplay(); // 新增：更新用户信息显示（头像+姓名+性别）

    Ui::Widget *ui;
    Record *m_recordPage;
    Manage *m_managePage;
    Budget *m_budgetPage;
    datachartwidget *m_chartPage;
    QString currentViewingMonth;
    QMap<QString, MonthStat> allMonthsData;
    QString m_bookName;

    // 用户信息区域
    QWidget *m_userInfoWidget;   // 用户信息容器
    QLabel *m_avatarLabel;
    QPushButton *m_avatarBtn;
    QLabel *m_nameLabel;         // 姓名标签
    QLabel *m_genderLabel;       // 性别标签

    // 用户数据
    QString m_userId;
    QString m_userName;
    int m_userAge;
    QString m_userGender;
    QString m_userSignature;
    QPixmap m_userAvatar;
};

#endif // WIDGET_H