#include "statrender.h"
#include <QColor>
#include <QFont>
#include <QPen>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDebug>

HistogramPlot::HistogramPlot(QWidget *parent) : QWidget(parent) {
    plot = new QwtPlot(this);
    plot->setAxisAutoScale(QwtPlot::xBottom);
    plot->setAxisAutoScale(QwtPlot::yLeft);
    plot->setCanvasBackground(Qt::white);
    //plot->setAxisScale(QwtPlot::xBottom, -1, 10);
    //plot->setAxisScale(QwtPlot::yLeft, -2, 2);
    //plot->setCanvasBackground(QColor(255,255,255,255));

    //QwtPlotPanner *panner = new QwtPlotPanner(plot->canvas());
    //panner->setMouseButton(Qt::MidButton);

    plotGrid = new QwtPlotGrid();
    plotGrid->enableXMin(true);
    plotGrid->enableYMin(true);
    plotGrid->setMajPen(QPen(Qt::black, 1, Qt::DotLine));
    plotGrid->setMinPen(QPen(Qt::gray, 0, Qt::DotLine));
    plotGrid->attach(plot);

    /*QwtPlotZoomer* zoomer = new plotZoomer(plot->canvas());
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);*/

    QFont labelFont;
    labelFont.setBold(true);
    mLabel.setColor(Qt::black);
    mLabel.setFont(labelFont);

    mLabel.setText(QString());
    mX = new QwtPlotMarker();
    mX->setLineStyle(QwtPlotMarker::HLine);
    mX->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    mX->setYValue(0.0);
    mX->setLabel(mLabel);
    mX->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
    mX->attach(plot);

    mLabel.setText(QString());
    mY = new QwtPlotMarker();
    mY->setLineStyle(QwtPlotMarker::VLine);
    mY->setLinePen(QPen(Qt::gray, 2, Qt::SolidLine));
    mY->setXValue(0.0);
    mY->setLabel(mLabel);
    mY->setLabelAlignment(Qt::AlignTop | Qt::AlignRight);
    mY->attach(plot);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(plot);
    hlayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(hlayout);

    showLegend();
}

HistogramPlot::~HistogramPlot() {
}

void HistogramPlot::setXLabel(const QString &label) {
    mLabel.setText(label);
    mX->setLabel(mLabel);
    plot->replot();
}

void HistogramPlot::setYLabel(const QString &label) {
    mLabel.setText(label);
    mY->setLabel(mLabel);
    plot->replot();
}

void HistogramPlot::showLegend() {
    plot->insertLegend(new QwtLegend(plot), QwtPlot::BottomLegend);
}

void HistogramPlot::setParams(const QList<QString> &names, const QList<QList<double> > &params) {
    plot->detachItems(QwtPlotItem::Rtti_PlotHistogram);

    for(int i=0; i<names.size(); i++) {
        QwtPlotHistogram *h = new QwtPlotHistogram(names[i]);
        QVector<QwtIntervalSample> hdata;
        for(int j=0; j<params.size(); j++) {
            hdata.append(QwtIntervalSample(params[j][i], j*names.size()+i, j*names.size()+i+1));
        }
        h->setSamples(hdata);
        h->setBrush(QBrush( (Qt::GlobalColor)((int)Qt::red+i%12) ));
        h->setPen(QPen( (Qt::GlobalColor)((int)Qt::red+i%12) ));
        h->attach(plot);
    }

    plot->replot();
}

/********************************************************************/

StatRender::StatRender(MainWindow *parent) : QWidget(parent) {
    hplot = new HistogramPlot(this);
    hplot->setYLabel("p.");
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(hplot);
    hlayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(hlayout);
}

void StatRender::drawStats(const QDateTime &s, const QDateTime &e, Type type, Div div) {
    MainWindow *mw = static_cast<MainWindow*>(parent());
    if(!mw) return;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "tmp");
    db.setDatabaseName("cheques.db");
    if(!db.open()) qDebug() << db.lastError();
    //QSqlDatabase &db = mw->getDatabase();
    QString baseReq;
    QList<QString> names;
    switch(type) {
        case Goods: {
            baseReq = "SELECT goods.name, SUM(goods.cost) FROM goods INNER JOIN (SELECT goods_in_cheque.gid FROM cheques INNER JOIN goods_in_cheque ON goods_in_cheque.cid = cheques.id WHERE cheques.date>=%1 AND cheques.date<=%2) AS content ON goods.id = content.gid WHERE content.gid = goods.id GROUP BY goods.name;";
            QSqlQuery q(db);
            q.exec(QString("SELECT * FROM goods"));
            while(q.next()) names.append(q.value(1).toString());
            break;
        }
        case Categories: {
            baseReq = "SELECT category.name, SUM(costs.cost) FROM category INNER JOIN (SELECT goods.cost, goods.category_id FROM goods INNER JOIN (SELECT goods_in_cheque.gid FROM cheques INNER JOIN goods_in_cheque ON goods_in_cheque.cid = cheques.id WHERE cheques.date >=%1 AND cheques.date<=%2) AS content ON goods.id = content.gid WHERE content.gid = goods.id) AS costs ON costs.category_id = category.id WHERE costs.category_id = category.id GROUP BY category.name;";
            QSqlQuery q(db);
            q.exec(QString("SELECT * FROM category"));
            while(q.next()) names.append(q.value(1).toString());
            break;
        }
    }

    qint64 timestep;
    QString ds;
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(0);
    switch(div) {
        case Day: dt = dt.addDays(1); ds = tr("days"); break;
        case Week: dt = dt.addDays(7); ds = tr("weeks"); break;
        case Month: dt = dt.addDays(30); ds = tr("months"); break;
        case Year: dt = dt.addYears(1); ds = tr("years"); break;
        default: break;
    }
    timestep = div == None ? e.toTime_t()-s.toTime_t() : dt.toTime_t();

    QSqlQuery q(db);
    QList<QList<double> > stats;
    int counter = 0;
    for(qint64 i=s.toTime_t(); i<e.toTime_t(); i+=timestep, counter++) {
        stats.append(QList<double>());
        for(int j=0; j<names.size(); j++) stats[counter].append(0);
        if(!q.exec(baseReq.arg(i).arg(i+timestep))) qDebug() << q.lastError();
        while(q.next()) {
            for(int j=0; j<names.size(); j++) {
                if(names[j] == q.value(0).toString()) {
                    stats[counter][j] = q.value(1).toDouble();
                    break;
                }
            }
        }
    }

    db.close();

    hplot->setXLabel(ds);
    hplot->setParams(names, stats);

}
