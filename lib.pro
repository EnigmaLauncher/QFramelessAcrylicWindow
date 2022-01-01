TARGET = $$qtLibraryTarget(QAcrylicFramelessHelper)
TEMPLATE = lib
win32: DLLDESTDIR = $$OUT_PWD/bin
else: unix: DESTDIR = $$OUT_PWD/bin
QT += gui-private widgets
CONFIG += c++17 strict_c++ utf8_source warn_on

DEFINES += \
    QT_NO_CAST_FROM_ASCII \
    QT_NO_CAST_TO_ASCII \
    QT_NO_KEYWORDS \
    QT_DEPRECATED_WARNINGS \
    QT_DISABLE_DEPRECATED_BEFORE=0x060200 \
    QTACRYLICHELPER_BUILD_LIBRARY \
    FRAMELESSHELPER_BUILD_LIBRARY
RESOURCES += \
    $$PWD/src/qtacrylichelper.qrc \
    src/qtacrylichelper.qrc
HEADERS += \
    $$PWD/src/framelesshelper_global.h \
    $$PWD/src/framelesshelper.h \
    $$PWD/src/framelesswindowsmanager.h \
    $$PWD/src/utilities.h \
    $$PWD/src/qtacrylichelper_global.h \
    $$PWD/src/qtacryliceffecthelper.h \
    $$PWD/src/qtacrylicutilities.h \
    $$PWD/src/qtacrylicwidget.h
SOURCES += \
    $$PWD/src/framelesshelper.cpp \
    $$PWD/src/framelesswindowsmanager.cpp \
    $$PWD/src/utilities.cpp \
    $$PWD/src/qtacrylicutilities.cpp \
    $$PWD/src/qtacryliceffecthelper.cpp \
    $$PWD/src/qtacrylicwidget.cpp
qtHaveModule(quick) {
    QT += quick
    HEADERS += \
        $$PWD/src/framelessquickhelper.h \
        $$PWD/src/qtacrylicitem.h
    SOURCES += \
        $$PWD/src/framelessquickhelper.cpp \
        $$PWD/src/qtacrylicitem.cpp
}
win32 {
    HEADERS += \
        $$PWD/src/framelesshelper_windows.h \
        $$PWD/src/framelesshelper_win32.h \
        $$PWD/src/qtacryliceffecthelper_win32.h
    SOURCES += \
        $$PWD/src/utilities_win32.cpp \
        $$PWD/src/framelesshelper_win32.cpp \
        $$PWD/src/qtacryliceffecthelper_win32.cpp \
        $$PWD/src/qtacrylicutilities_win32.cpp
    LIBS += -luser32 -lshell32 -ldwmapi -lwinmm -ldwrite -lole32
    RC_FILE = $$PWD/src/qacrylicframelesshelper.rc
}
