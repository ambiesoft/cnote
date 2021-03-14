TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    cnote/cnote.cpp \
    ../../lsMisc/GetChildWindowBy.cpp \
    ../../lsMisc/GetLastErrorString.cpp \
    ../../lsMisc/UTF16toUTF8.cpp \
    ../../lsMisc/FindTopWindowFromPID.cpp \
    ../../lsMisc/I18N.cpp \
    ../../lsMisc/OpenCommon.cpp \
    ../../lsMisc/stdosd/stdosd.cpp \
    ../../lsMisc/stdosd/stdosd_win.cpp

HEADERS += \
    cnote/stdafx.h \
    ../../lsMisc/GetChildWindowBy.h \
    ../../lsMisc/GetLastErrorString.h \
    ../../lsMisc/UTF16toUTF8.h \
    ../../lsMisc/FindTopWindowFromPID.h \
    ../../lsMisc/I18N.h \
    ../../lsMisc/OpenCommon.h \
    ../../lsMisc/stdosd/stdosd.h

INCLUDEPATH += ./cnote

win32 {
    message("win32")
    HEADERS +=

    SOURCES +=

    win32-g++ {
        message("win32-g++")
    }
    win32-msvc* {
        message("win32-msvc*")
        LIBS += User32.lib shell32.lib
    }
}
