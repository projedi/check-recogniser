#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QGridLayout>
#include <QFont>
#include <QtSql>
#include <QMessageBox>

#include "statwindow.h"
#include "lcdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createActions();
    createMenus();

    dbViewModel = new QStandardItemModel(1, 3, this);
    dbViewModel->setHeaderData(0, Qt::Horizontal, tr("Good"), Qt::DisplayRole);
    dbViewModel->setHeaderData(1, Qt::Horizontal, tr("Cost"), Qt::DisplayRole);
    dbViewModel->setHeaderData(2, Qt::Horizontal, tr("Category"), Qt::DisplayRole);

    tvDBView = new ExtTreeView(this);
    tvDBView->setModel(dbViewModel);
    connect(tvDBView, SIGNAL(loadChequeRequested()), this, SLOT(loadCheque()));
    connect(tvDBView, SIGNAL(deleteChequeRequested(int)), this, SLOT(deleteCheque(int)));

    lbInfo = new QLabel("<b>0</b>", this);

    cRecognizer = new ChequeRecognizer(this);

    QPushButton *pbLoadCheque = new QPushButton(tr("Load cheque"), this);
    QPushButton *pbShowStats = new QPushButton(tr("Show stats"), this);
    connect(pbLoadCheque, SIGNAL(clicked()), this, SLOT(loadCheque()));
    connect(pbShowStats, SIGNAL(clicked()), this, SLOT(showStats()));

    cbPeriod = new QComboBox(this);
    cbPeriod->addItem(tr("week"));
    cbPeriod->addItem(tr("month"));
    cbPeriod->addItem(tr("year"));
    cbPeriod->addItem(tr("all time"));
    cbPeriod->setCurrentIndex(0);
    connect(cbPeriod, SIGNAL(currentIndexChanged(int)), this, SLOT(showCheques(int)));

    QLabel *lb1 = new QLabel(tr("Cheques in database: "), this);
    QLabel *lb2 = new QLabel(tr("Period: "), this);

    QGridLayout *hlayout = new QGridLayout();
    hlayout->addWidget(pbLoadCheque, 0, 0);
    hlayout->addWidget(pbShowStats, 0, 1);
    hlayout->addWidget(lb2, 0, 3);
    hlayout->addWidget(cbPeriod, 0, 4);
    hlayout->setColumnStretch(2, 1);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSpacing(5);

    QWidget *w = new QWidget(this);
    QGridLayout *mlayout = new QGridLayout();
    mlayout->addLayout(hlayout, 0, 0, 1, 3);
    mlayout->addWidget(tvDBView, 1, 0, 1, 3);
    mlayout->addWidget(lb1, 2, 0);
    mlayout->addWidget(lbInfo, 2, 1);
    mlayout->setColumnStretch(0, 0);
    mlayout->setColumnStretch(1, 0);
    mlayout->setColumnStretch(2, 1);
    mlayout->setRowStretch(1, 1);
    mlayout->setContentsMargins(5, 5, 5, 5);
    mlayout->setSpacing(5);
    w->setLayout(mlayout);
    this->setCentralWidget(w);
    this->setWindowTitle(tr("Cheque recognizer"));
    this->resize(700, 450);

    cdb = QSqlDatabase::addDatabase("QSQLITE");
    bool needCreation = !QFile::exists("cheques.db");
    cdb.setDatabaseName("cheques.db");
    if(!cdb.open()) qDebug() << "Unable to open database";
    if(needCreation && cdb.isOpen()) {
        QSqlQuery q(cdb);
        if(!q.exec(QString("CREATE TABLE cheques (id INTEGER PRIMARY KEY AUTOINCREMENT, date DATETIME, total DOUBLE);"))) qDebug() << q.lastError();
        if(!q.exec(QString("CREATE TABLE category (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT);"))) qDebug() << q.lastError();
        if(!q.exec(QString("CREATE TABLE goods (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, cost DOUBLE, category_id INTEGER, FOREIGN KEY(category_id) REFERENCES category(id));"))) qDebug() << q.lastError();
        if(!q.exec(QString("CREATE TABLE goods_in_cheque (cid INTEGER, gid INTEGER, count INTEGER, PRIMARY KEY(cid, gid), FOREIGN KEY(cid) REFERENCES cheques(id), FOREIGN KEY(gid) REFERENCES goods(id));"))) qDebug() << q.lastError();
    } else if(cdb.isOpen()) {
        QSqlQuery q(cdb);
        if(!q.exec(QString("SELECT * FROM category"))) qDebug() << q.lastError();
        while(q.next()) categoryNames.insert(q.value(0).toInt(), q.value(1).toString());
    }

    updateChequesRowCount();
    showCheques(cbPeriod->currentIndex());
}

MainWindow::~MainWindow() {
    cdb.close();
}

//-----------------------------------------------------------------

void MainWindow::createActions() {
    //actLoadCheque = new QAction(tr("Load cheque"), this);
    actLoadCheque = menuBar()->addAction(tr("Load cheque"));
    connect(actLoadCheque, SIGNAL(triggered()), this, SLOT(loadCheque()));

    actShowStats = menuBar()->addAction(tr("Statistics"));
    connect(actShowStats, SIGNAL(triggered()), this, SLOT(showStats()));
}

void MainWindow::createMenus() {
}

//-----------------------------------------------------------------

void MainWindow::updateChequesRowCount() {
    QSqlQuery q(cdb);
    if(!q.exec("SELECT Count(*) FROM cheques;")) return;
    q.next();
    lbInfo->setText(QString("<b>%1</b>").arg(q.value(0).toInt()));
}

void MainWindow::showCheques(int type) {
    QDateTime t = QDateTime::currentDateTime();
    switch(type) {
        case 0: t.setDate(QDate::currentDate().addDays(-7)); break;
        case 1: t.setDate(QDate::currentDate().addMonths(-1)); break;
        case 2: t.setDate(QDate::currentDate().addYears(-1)); break;
        default: break;
    }
    qint64 date = type == 3 ? 0 : t.toTime_t();

    QSqlQuery q(cdb);
    if(!q.exec(QString("SELECT * FROM cheques WHERE date>%1").arg(date))) qDebug() << q.lastError();
    //dbViewModel->clear();
    dbViewModel->setRowCount(0);
    int pos = 0;

    while(q.next()) {
        QString bdate = QDateTime::fromTime_t(q.value(1).toUInt()).toString("dd.MM.yyyy hh:mm");
        QString tcost = QString::number(q.value(2).toDouble());
        dbViewModel->insertRows(pos, 1);
        dbViewModel->setData(dbViewModel->index(pos, 0), q.value(0), Qt::UserRole);
        dbViewModel->setData(dbViewModel->index(pos, 0), tr("Date: %1").arg(bdate), Qt::DisplayRole);
        dbViewModel->setData(dbViewModel->index(pos, 1), tr("Total cost: %1").arg(tcost), Qt::DisplayRole);

        QSqlQuery cq(cdb);
        //if(!cq.exec(QString("SELECT goods.name, goods.cost, content.count FROM goods INNER JOIN (SELECT * FROM goods_in_cheque WHERE cid=%1) AS content ON goods.id=content.gid;").arg(q.value(0).toInt()))) qDebug() << q.lastError();
        //if(!cq.exec(QString("SELECT goods.name, goods.cost, goods_in_cheque.count FROM goods INNER JOIN goods_in_cheque ON goods.id=goods_in_cheque.gid WHERE goods_in_cheque.cid=%1;").arg(q.value(0).toInt()))) qDebug() << q.lastError();
        if(!cq.exec(QString("SELECT goods.name, goods.cost, goods.category_id FROM goods INNER JOIN goods_in_cheque ON goods.id=goods_in_cheque.gid WHERE goods_in_cheque.cid=%1;").arg(q.value(0).toInt()))) qDebug() << q.lastError();
        QModelIndex index = dbViewModel->index(pos, 0);
        int ipos = 0;
        dbViewModel->insertColumns(0, 3, index);
        while(cq.next()) {
            bool res = dbViewModel->insertRows(ipos, 1, index);
            res = dbViewModel->setData(dbViewModel->index(ipos, 0, index), cq.value(0).toString(), Qt::DisplayRole);
            res = dbViewModel->setData(dbViewModel->index(ipos, 1, index), cq.value(1).toDouble(), Qt::DisplayRole);
            res = dbViewModel->setData(dbViewModel->index(ipos, 2, index), categoryNames.value(cq.value(2).toInt()), Qt::DisplayRole);
            ipos++;
        }
        pos++;
    }

    tvDBView->resizeColumnToContents(0);
    tvDBView->resizeColumnToContents(1);
    tvDBView->setColumnWidth(0, tvDBView->columnWidth(0)+15);
}

//-----------------------------------------------------------------

void MainWindow::loadCheque() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load new cheque"), ".", "Images (*.png *.jpg *.jpeg *.tif *.tiff)");
    if(fileName.isEmpty()) return;
    LCDialog *lcDialog = new LCDialog(fileName, this);
    if(lcDialog->exec() == QDialog::Accepted) {
        Cheque ch = lcDialog->collectData();
        if(ch.goods.isEmpty()) {
            lcDialog->deleteLater();
            return;
        }

        QSqlQuery q(cdb);
        if(!q.exec(QString("INSERT INTO cheques (date, total) VALUES(%1, %2);").arg(ch.date.toTime_t()).arg(ch.total))) qDebug() << q.lastError();
        if(!q.exec("SELECT last_insert_rowid();")) qDebug() << q.lastError();
        q.next();
        int chId = q.value(0).toInt();

        for(int i=0; i<ch.goods.size(); i++) {
            int gcid = categoryNames.key(ch.goods[i].category, -1);
            if(gcid == -1) {
                if(!q.exec(QString("INSERT INTO category (name) VALUES (\"%1\");").arg(ch.goods[i].category))) qDebug() << q.lastError();
                q.exec("SELECT last_insert_rowid();");
                q.next();
                gcid = q.value(0).toInt();
                categoryNames.insert(gcid, ch.goods[i].category);
            }

            int gid = 0;
            if(!q.exec(QString("SELECT id FROM goods WHERE name=\"%1\" AND cost=%2 AND category_id=%3;").arg(ch.goods[i].name).arg(ch.goods[i].cost).arg(gcid))) qDebug() << q.lastError();
            if(!q.next()) {
                if(!q.exec(QString("INSERT INTO goods (name, cost, category_id) VALUES (\"%1\", %2, %3);").arg(ch.goods[i].name).arg(ch.goods[i].cost).arg(gcid))) qDebug() << q.lastError();
                q.exec("SELECT last_insert_rowid();");
                q.next();
                gid = q.value(0).toInt();
            } else {
                gid = q.value(0).toInt();
            }

            if(!q.exec(QString("INSERT INTO goods_in_cheque VALUES (%1, %2, %3);").arg(chId).arg(gid).arg(ch.goods[i].count))) qDebug() << q.lastError();
        }

        updateChequesRowCount();
        showCheques(cbPeriod->currentIndex()); //need replace it
    }

    lcDialog->deleteLater();
}

void MainWindow::showStats() {
    StatWindow *sw = new StatWindow(this);
    sw->exec();
    sw->deleteLater();
}

//-----------------------------------------------------------------

void MainWindow::deleteCheque(int pos) {
    QString chId = dbViewModel->data(dbViewModel->index(pos, 0), Qt::UserRole).toString();

    QSqlQuery q(cdb);
    q.exec(QString("DELETE FROM cheques WHERE id=%1;").arg(chId));
    if(!q.exec(QString("DELETE FROM goods_in_cheque WHERE cid=%1;").arg(chId))) qDebug() << q.lastError();
    dbViewModel->removeRow(pos);
}

//-----------------------------------------------------------------

int MainWindow::getCategoryIDForGood(const QString &goodName) {
    QSqlQuery q(cdb);
    if(!q.exec(QString("SELECT category_id FROM goods WHERE name=\"%1\";").arg(goodName))) qDebug() << q.lastError();
    if(!q.next()) return -1;
    return q.value(0).toInt();
}

//-----------------------------------------------------------------

QSqlDatabase &MainWindow::getDatabase() {
    return cdb;
}

QMap<int, QString> &MainWindow::getCategoryNames() {
    return categoryNames;
}

ChequeRecognizer *MainWindow::getRecognizer() {
    return cRecognizer;
}

/*******************************************************************/

ExtTreeView::ExtTreeView(QWidget *parent) : QTreeView(parent) {
    actDeleteCheque = new QAction(tr("Delete cheque"), this);
    actNewCheque = new QAction(tr("Load cheque"), this);
    connect(actDeleteCheque, SIGNAL(triggered()), this, SLOT(slotDeleteChequeRequested()));
    connect(actNewCheque, SIGNAL(triggered()), this, SLOT(slotLoadChequeRequested()));

    cMenu = new QMenu(this);
    cMenu->addAction(actNewCheque);
    cMenu->addAction(actDeleteCheque);
}

void ExtTreeView::contextMenuEvent(QContextMenuEvent *event) {
    cInx = this->indexAt(event->pos());
    if(cInx.parent().isValid()) cInx = cInx.parent();
    if(cInx.isValid()) actDeleteCheque->setEnabled(true);
    cMenu->exec(event->globalPos());
}

void ExtTreeView::slotLoadChequeRequested() {
    emit loadChequeRequested();
}

void ExtTreeView::slotDeleteChequeRequested() {
    QString date = cInx.model()->data(cInx.model()->index(cInx.row(), 0)).toString();
    int res = QMessageBox::question(this, tr("Cheque deletion"), tr("Do you really want to delete cheque %1?").arg(date.right(date.size()-6)), QMessageBox::Yes, QMessageBox::No);
    if(res == QMessageBox::Yes) emit deleteChequeRequested(cInx.row());
}
