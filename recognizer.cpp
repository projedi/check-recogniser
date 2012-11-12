#include "recognizer.h"

ChequeRecognizer::ChequeRecognizer(QObject *parent) : QObject(parent) {
}

Cheque ChequeRecognizer::recognizeFile(const QString &fileName) {
    Cheque ch;
    ch.date = QDateTime::currentDateTime();
    ch.total = 40.0;
    ch.goods.append(Good("milk", 40.0, 1));
    return ch;
}
