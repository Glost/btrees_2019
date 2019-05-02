#-------------------------------------------------
#
# Project created by QtCreator 2019-04-30T23:16:56
#
#-------------------------------------------------

include($$PWD/../../SQLiteStudio3/plugins.pri)

QT += printsupport

TARGET = BtreesMods
TEMPLATE = lib

DEFINES += BTREESMODS_LIBRARY

SOURCES += \
        btreesmods.cpp

HEADERS += \
        btreesmods.h \
        btreesmods_global.h 

OTHER_FILES += \
    btreesmods.json
