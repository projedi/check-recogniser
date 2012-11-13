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
    recognizer.cpp \
    statrender.cpp \
    statwindow.cpp

HEADERS  += mainwindow.h \
    lcdialog.h \
    previewer.h \
    recognizer.h \
    statrender.h \
    statwindow.h

INCLUDEPATH += C:/libs/qwt-6.0.1/include
LIBS += -lqwtd -llept -ltesseract

TRANSLATIONS += trans.ts

RESOURCES += \
    resources.qrc
