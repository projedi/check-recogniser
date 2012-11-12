#ifndef PREVIEWER_H
#define PREVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QResizeEvent>

class ImagePreviewer : public QGraphicsView {
    Q_OBJECT

public:
    ImagePreviewer(QWidget *parent = 0);

    void showImage(const QString &fileName);
    void hideImage();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QGraphicsTextItem *gtiNoPreview;
    QGraphicsPixmapItem *gpiImage;
    QString currentPath;

    void adjustImage();
};

#endif // PREVIEWER_H
