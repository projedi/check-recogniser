#include "recognizer.h"

#include <QSettings>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

#include <QtCore/QTextCodec>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#define SETTING_ORGANIZATION "qt-box-editor"
#define SETTING_APPLICATION "QT Box Editor"

QString getDataPath() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       SETTING_ORGANIZATION, SETTING_APPLICATION);
    QString dataPath;
    if (settings.contains("Tesseract/DataPath")) {
      dataPath = settings.value("Tesseract/DataPath").toString();
    }
    return dataPath;
}

ChequeRecognizer::ChequeRecognizer(QObject *parent) : QObject(parent) {
}

Cheque ChequeRecognizer::recognizeFile(const QString &fileName) {
    Cheque ch;

    tesseract::TessBaseAPI tessApi;

     #ifdef _WIN32
     QString envQString = "TESSDATA_PREFIX=" + getDataPath() ;
     QByteArray byteArrayWin = envQString.toUtf8();
     const char * env = byteArrayWin.data();
     putenv(env);
     #else
     QByteArray byteArray1 = getDataPath().toUtf8();
     const char * datapath = byteArray1.data();
     setenv("TESSDATA_PREFIX", datapath, 1);
     #endif

    tessApi.Init(NULL, "rus");
    PIX *pix = pixRead(fileName.toLocal8Bit());
    tessApi.SetImage(pix);

    char *text = tessApi.GetUTF8Text();

    QString txt = QString::fromUtf8(text);

    QStringList str;
    str = txt.split("\n");

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8")); //cyrillic .cpp
    QRegExp rgood("[0-9]+[,.][0-9][0-9].[0-9]+[,.][0-9][0-9]$");
    QRegExp rcost("[0-9]+[,.][0-9][0-9]$");
    QRegExp rname("[0-9,.]*[хХxX]");
    QRegExp rcount("[хХxX]");
    QRegExp rtotal("ИТОГ");
    QRegExp rtotalc("[0-9]+[,.][0-9][0-9]");
    QRegExp rdatetime("[0-9][0-9]-[0-9][0-9]-[0-9][0-9] [0-9][0-9]:[0-9][0-9]");


    QTextStream out(stdout);

    int total = 0;
    for (QStringList::Iterator it = str.begin(); it != str.end(); ++it) {
        if ((*it).contains(rgood)) {
            int index1 = (*it).indexOf(rgood);
            int index2 = (*it).indexOf(rcost);
            int index3 = (*it).indexOf(rname);
            int index4 = (*it).indexOf(rcount);

            QString gname = (*it).mid(0,index3);
            gname = gname.replace('"', '\'');
            double gcost = (*it).mid(index1, index2-index1).replace(',', '.').toDouble();
            double gcount = (*it).mid(index3, index4-index3).replace(',', '.').toDouble();
//            out << (*it).mid(0,index1) << endl;
//            out << (*it).mid(index2, -1) << endl;
//            out << *it << endl;
//            out << (*it).mid(index3, index4-index3).replace(',', '.') << endl;
//            out << gname << endl;
//            out << gcost << endl;
//            out << gcount << endl;
            ch.goods.append(Good(gname, gcost, gcount));
            total += gcost*gcount;
        }
        if ((*it).contains(rtotal)) {
            rtotalc.indexIn(*it);
            int index1 = (*it).indexOf(rtotalc);
            total = (*it).mid(index1, rtotalc.matchedLength()).replace(',', '.').toDouble();
//            out << (*it).mid(index1, rtotalc.matchedLength()).replace(',', '.') << endl;
        }
        if ((*it).contains(rdatetime)) {
            rdatetime.indexIn(*it);
            int index1 = (*it).indexOf(rdatetime);
            QString datetime = (*it).mid(index1, rdatetime.matchedLength());
            ch.date = QDateTime::fromString(datetime, "dd-MM-yy hh:mm");
        }
    }
    ch.total = total;

//    ch.date = QDateTime::fromString("13-11-12 19:26", "dd-MM-yy hh:mm");

    pixDestroy(&pix);
    delete [] text;

//    ch.date = QDateTime::currentDateTime();
//    ch.total = 40.0;
//    ch.goods.append(Good("milk", 40.0, 1));
    return ch;
}
