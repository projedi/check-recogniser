#ifndef STATWINDOW_H
#define STATWINDOW_H

#include <QDialog>
#include <QDateEdit>
#include "statrender.h"

class StatWindow : public QDialog {
    Q_OBJECT

public:
    StatWindow(MainWindow *parent = 0);
    ~StatWindow();

private:
    StatRender *sr;
    QComboBox *cbType, *cbDiv;
    QDateEdit *deBeg, *deEnd;

private slots:
    void showStats();
};

#endif // STATWINDOW_H
