QT += widgets
QT += core gui network
CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    accountingserver.cpp \
    addone.cpp \
    bookdetail.cpp \
    budget.cpp \
    createbookdialog.cpp \
    editbookdialog.cpp \
    editrecorddialog.cpp \
    logindialog.cpp \
    main.cpp \
    manage.cpp \
    multibookdialog.cpp \
    networkclient.cpp \
    record.cpp \
    tagmanagerdialog.cpp \
    widget.cpp

HEADERS += \
    accountingserver.h \
    addone.h \
    bookdetail.h \
    budget.h \
    createbookdialog.h \
    editbookdialog.h \
    editrecorddialog.h \
    logindialog.h \
    manage.h \
    multibookdialog.h \
    networkclient.h \
    record.h \
    tagmanagerdialog.h \
    widget.h

FORMS += widget.ui \
    addone.ui \
    budget.ui \
    record.ui

TRANSLATIONS += \
    accountbook_en_GB.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc \