#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QObject>
#include <QDateTime>
#include <QList>

#define F_NAME      0
#define F_COST      1
#define F_COUNT     2
#define F_CATEGORY  3

#define FV_GOOD     0
#define FV_UNSURE   1
#define FV_BAD      2

struct Good {
    Good(const QString &_name = "", double _cost = 0.0, double _count = 1, const QString &_category = "")
        : name(_name), cost(_cost), count(_count), category(_category) {
        for(int i=0; i<4; i++) flags[i] = FV_GOOD;
    }

    QString name;
    double cost;
    double count;
    QString category;
    int flags[4];
};

struct Cheque {
    QDateTime date;
    QList<Good> goods;
    double total;
};

//----------------------------------------------------------------

#include "tesseract/baseapi.h"

class ChequeRecognizer : public QObject {
public:
    ChequeRecognizer(QObject *parent = 0);

    Cheque recognizeFile(const QString &fileName, bool *ok = 0);

private:
    int prepareData(const QString &fileName);
    void cleanUp(int fcount);
    QString recognizeString(const QString &fileName);

    tesseract::TessBaseAPI tessApi;
};

#endif // RECOGNIZER_H
