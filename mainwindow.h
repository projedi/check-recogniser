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

class ExtTreeView : public QTreeView {
    Q_OBJECT

public:
    ExtTreeView(QWidget *parent = 0);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    QAction *actDeleteCheque, *actNewCheque;
    QMenu *cMenu;
    QModelIndex cInx;

private slots:
    void slotDeleteChequeRequested();
    void slotLoadChequeRequested();

signals:
    void deleteChequeRequested(int row);
    void loadChequeRequested();
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSqlDatabase &getDatabase();
    QMap<int, QString> &getCategoryNames();
    ChequeRecognizer *getRecognizer();

    int getCategoryIDForGood(const QString &goodName);

private:
    QAction *actLoadCheque, *actShowStats;
    ExtTreeView *tvDBView;
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
    void deleteCheque(int pos);
    void showStats();
    void showCheques(int type);
};

#endif // MAINWINDOW_H
