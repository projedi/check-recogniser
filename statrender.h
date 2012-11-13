#ifndef STATRENDER_H
#define STATRENDER_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_panner.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_symbol.h>
#include <qwt_plot_histogram.h>

#include "mainwindow.h"

class HistogramPlot : public QWidget {
    Q_OBJECT

public:
    HistogramPlot(QWidget *parent = 0);
    ~HistogramPlot();
    void setXLabel(const QString &label);
    void setYLabel(const QString &label);
    void showLegend();

    void setParams(const QList<QString> &names, const QList<QList<double> > &params);

    QwtPlot *plot;

private:
    QwtPlotMarker *mY, *mX;
    QwtPlotGrid *plotGrid;
    QwtText mLabel;
};

class StatRender : public QWidget {
    Q_OBJECT

public:
    enum Type { Goods, Categories };
    enum Div { Day, Week, Month, Year, None };

    StatRender(MainWindow *parent = 0);

    void drawStats(const QDateTime &s, const QDateTime &d, Type type, Div div);

private:
    HistogramPlot *hplot;
};

#endif // STATRENDER_H
