#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QObject>
#include <QDateTime>
#include <QList>

//#include "tesseract/baseapi.h"
//#include "leptonica/allheaders.h"

struct Good {
    Good(const QString &_name = "", double _cost = 0.0, double _count = 1, const QString &_category = "")
        : name(_name), cost(_cost), count(_count), category(_category) {}

    QString name;
    double cost;
    double count;
    QString category;
    char flags[4];
};

struct Cheque {
    QDateTime date;
    QList<Good> goods;
    double total;
};

class ChequeRecognizer : public QObject {
public:
    ChequeRecognizer(QObject *parent = 0);

    Cheque recognizeFile(const QString &fileName);
};

#endif // RECOGNIZER_H
