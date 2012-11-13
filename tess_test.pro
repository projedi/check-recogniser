#-------------------------------------------------
#
# Project created by QtCreator 2012-11-12T11:28:14
#
#-------------------------------------------------

QT       += core gui sql

TARGET = cheque_recognizer_test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    previewer.cpp \
    lcdialog.cpp \
    recognizer.cpp

HEADERS  += mainwindow.h \
    lcdialog.h \
    previewer.h \
    recognizer.h

LIBS += -llept -ltesseract

TRANSLATIONS += trans.ts

RESOURCES += \
    resources.qrc
