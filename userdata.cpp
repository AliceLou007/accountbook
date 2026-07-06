#include "userdata.h"
#include "networkclient.h"
#include <QDir>
#include <QRegularExpression>

QString UserData::currentUserId()
{
    QString userId = NetworkClient::getInstance()->currentUserId();
    if (userId.isEmpty()) {
        userId = "default";
    }
    return userId;
}

QString UserData::safeUserId()
{
    QString userId = currentUserId();
    userId.replace(QRegularExpression("[^A-Za-z0-9_-]"), "_");
    if (userId.isEmpty()) {
        userId = "default";
    }
    return userId;
}

QString UserData::rootDir()
{
    QString dirPath = QDir::currentPath() + "/data/" + safeUserId();
    QDir().mkpath(dirPath);
    return dirPath;
}

QString UserData::path(const QString &fileName)
{
    return rootDir() + "/" + fileName;
}

QString UserData::booksFile()
{
    return path("books.json");
}

QString UserData::selectedBookFile()
{
    return path("selected_book.json");
}

QString UserData::tagsFile()
{
    return path("tags.json");
}

QString UserData::recordFile(const QString &bookName)
{
    return path(QString("%1_data.txt").arg(bookName));
}

QString UserData::budgetFile(const QString &yearMonth, const QString &bookName)
{
    QString budgetDir = rootDir() + "/budget";
    QDir().mkpath(budgetDir);
    return budgetDir + QString("/%1_%2.json").arg(yearMonth, bookName);
}

QString UserData::imageDir(const QString &bookName)
{
    QString dirPath = rootDir() + "/images/" + bookName;
    QDir().mkpath(dirPath);
    return dirPath;
}

QString UserData::profileFile()
{
    return path("user_profile.json");
}

QString UserData::avatarFile()
{
    return path("avatar.png");
}
