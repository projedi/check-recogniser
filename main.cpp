#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("Windows-1251"));
    QApplication a(argc, argv);
    QTranslator t;
    t.load("./ru_RU.qm");
    a.installTranslator(&t);

    MainWindow w;
    w.show();
    
    return a.exec();
}
