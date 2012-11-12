#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTreeView>
#include <QLabel>
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QMap>
#include <QComboBox>

#include "recognizer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QMap<int, QString> &getCategoryNames();
    ChequeRecognizer *getRecognizer();

    int getCategoryIDForGood(const QString &goodName);

private:
    QAction *actLoadCheque, *actShowStats;
    QTreeView *tvDBView;
    QStandardItemModel *dbViewModel;
    QLabel *lbInfo;
    QComboBox *cbPeriod;

    QSqlDatabase cdb;
    ChequeRecognizer *cRecognizer;

    QMap<int, QString> categoryNames;

    void createActions();
    void createMenus();

    void updateChequesRowCount();

private slots:
    void loadCheque();
    void showStats();
    void showCheques(int type);
};

#endif // MAINWINDOW_H
