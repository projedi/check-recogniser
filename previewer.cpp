#include "previewer.h"

#include <QTextBlockFormat>
#include <QTextCursor>

ImagePreviewer::ImagePreviewer(QWidget *parent) : QGraphicsView(parent) {
    this->setScene(new QGraphicsScene(0, 0, 200, this->height(), this));

    QFont fb;
    fb.setBold(true);

    gtiNoPreview = new QGraphicsTextItem(tr("Preview\nunavailable"));
    gtiNoPreview->setFont(fb);
    gtiNoPreview->setDefaultTextColor(Qt::gray);
    gtiNoPreview->setTextWidth(gtiNoPreview->boundingRect().width());

    QTextBlockFormat format;
    format.setAlignment(Qt::AlignCenter);
    QTextCursor cursor = gtiNoPreview->textCursor();
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(format);
    cursor.clearSelection();
    gtiNoPreview->setTextCursor(cursor);

    scene()->addItem(gtiNoPreview);
    gtiNoPreview->setPos(width()/2.0-gtiNoPreview->boundingRect().width()/2.0, height()/2.0-gtiNoPreview->boundingRect().height()/2.0);

    gpiImage = 0;
    currentPath = "";
}

void ImagePreviewer::showImage(const QString &fileName) {
    if(currentPath == fileName) return;
    if(gpiImage) delete gpiImage;
    gpiImage = new QGraphicsPixmapItem(QPixmap(fileName));
    scene()->addItem(gpiImage);
    if(gtiNoPreview->scene() != 0) scene()->removeItem(gtiNoPreview);
    adjustImage();
    currentPath = fileName;
}

void ImagePreviewer::hideImage() {
    if(gpiImage) scene()->removeItem(gpiImage);
    scene()->addItem(gtiNoPreview);
}

void ImagePreviewer::resizeEvent(QResizeEvent *event) {
    this->scene()->setSceneRect(0, 0, event->size().width(), event->size().height());
    gtiNoPreview->setPos(width()/2.0-gtiNoPreview->boundingRect().width()/2.0, height()/2.0-gtiNoPreview->boundingRect().height()/2.0);
    if(gpiImage) adjustImage();
    QGraphicsView::resizeEvent(event);
}

void ImagePreviewer::adjustImage() {
    qreal sx = gpiImage->boundingRect().width()/width();
    qreal sy = gpiImage->boundingRect().height()/height();
    if(sx > 1.0 || sy > 1.0) gpiImage->setScale(qMin(1.0/sx, 1.0/sy));
    gpiImage->setPos(width()/2.0-gpiImage->boundingRect().width()*gpiImage->scale()/2.0, height()/2.0-gpiImage->boundingRect().height()*gpiImage->scale()/2.0);
}

