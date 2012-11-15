#include "statwindow.h"

#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

StatWindow::StatWindow(MainWindow *parent) : QDialog(parent) {
    QLabel *lb1 = new QLabel(tr("Statistics by: "), this);
    QLabel *lb2 = new QLabel(tr("division by: "), this);
    QLabel *lb3 = new QLabel(tr("| From: "), this);
    QLabel *lb4 = new QLabel(tr("to: "), this);

    cbType = new QComboBox(this);
    cbType->addItem(tr("category"), StatRender::Categories);
    cbType->addItem(tr("goods"), StatRender::Goods);
    cbDiv = new QComboBox(this);
    cbDiv->addItem(tr("day"), StatRender::Day);
    cbDiv->addItem(tr("week"), StatRender::Week);
    cbDiv->addItem(tr("month"), StatRender::Month);
    cbDiv->addItem(tr("year"), StatRender::Year);
    cbDiv->addItem(tr("none"), StatRender::None);
    cbDiv->setCurrentIndex(1);

    QPushButton *pbShowStat = new QPushButton(tr("Show"), this);
    connect(pbShowStat, SIGNAL(clicked()), this, SLOT(showStats()));

    deBeg = new QDateEdit(this);
    deBeg->setDate(QDate::currentDate().addDays(-7));
    deEnd = new QDateEdit(this);
    deEnd->setDate(QDate::currentDate());

    QGridLayout *hlayout = new QGridLayout();
    hlayout->addWidget(lb1, 0, 0);
    hlayout->addWidget(cbType, 0, 1);
    hlayout->addWidget(lb2, 0, 2);
    hlayout->addWidget(cbDiv, 0, 3);
    hlayout->addWidget(lb3, 0, 4);
    hlayout->addWidget(deBeg, 0, 5);
    hlayout->addWidget(lb4, 0, 6);
    hlayout->addWidget(deEnd, 0, 7);
    hlayout->addWidget(pbShowStat, 0, 9);
    hlayout->setColumnStretch(8, 1);
    hlayout->setSpacing(5);
    hlayout->setContentsMargins(0, 0, 0, 0);

    sr = new StatRender(parent);

    QGridLayout *mlayout = new QGridLayout();
    mlayout->addLayout(hlayout, 0, 0);
    mlayout->addWidget(sr, 1, 0);
    mlayout->setRowStretch(0, 0);
    mlayout->setRowStretch(1, 1);
    mlayout->setContentsMargins(5, 5, 5, 5);
    mlayout->setSpacing(5);
    this->setLayout(mlayout);

    this->setWindowTitle(tr("Statistics"));
    this->resize(750, 400);
}

StatWindow::~StatWindow() {
    sr->deleteLater();
}

void StatWindow::showStats() {
    if(deEnd->date() < deBeg->date()) {
        QMessageBox::warning(this, tr("Error"), tr("End date is less than start date"));
        return;
    }

    StatRender::Type t = (StatRender::Type)cbType->itemData(cbType->currentIndex()).toInt();
    StatRender::Div d = (StatRender::Div)cbDiv->itemData(cbDiv->currentIndex()).toInt();

    sr->drawStats(deBeg->dateTime(), deEnd->dateTime().addDays(1), t, d);
}
