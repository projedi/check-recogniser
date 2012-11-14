#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"

#include <QTextCodec>

int main(int argc, char *argv[]) {
    #ifdef __WIN32__
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("Windows-1251"));
    #endif
    QApplication a(argc, argv);
    QTranslator t;
    t.load(":/ru_RU.qm");
    a.installTranslator(&t);

    MainWindow w;
    w.show();
    
    return a.exec();
}
