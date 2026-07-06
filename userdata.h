#ifndef USERDATA_H
#define USERDATA_H

#include <QString>

class UserData
{
public:
    static QString currentUserId();
    static QString safeUserId();
    static QString rootDir();
    static QString path(const QString &fileName);
    static QString booksFile();
    static QString selectedBookFile();
    static QString tagsFile();
    static QString recordFile(const QString &bookName);
    static QString budgetFile(const QString &yearMonth, const QString &bookName);
    static QString imageDir(const QString &bookName);
    static QString profileFile();
    static QString avatarFile();
};

#endif // USERDATA_H
