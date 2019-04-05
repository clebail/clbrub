#include <QtDebug>
#include <QPainter>
#include <QResizeEvent>
#include "CMapView.h"

#define NBX             (RUBIKSIZE * 4)
#define NBY             (RUBIKSIZE * 3)
#define RB2             (RUBIKSIZE * 2)

CMapView::CMapView(QWidget *parent) : QWidget(parent) {
    rubik = nullptr;
}

void CMapView::setRubik(CRubik *rubik) {
    this->rubik = rubik;
    if(rubik != nullptr) {
        connect(rubik, SIGNAL(endRotate()), this, SLOT(onUpdate()));
        update();
    }
}

void CMapView::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    int i, j;

    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawRect(geometry());

    if(rubik != nullptr) {
        int x, y, z;

        for(i=0;i<NBY;i++) {
            for(j=0;j<NBX;j++) {
                painter.setBrush(Qt::white);

                if((j >= RUBIKSIZE && j < RB2) || (i >= RUBIKSIZE && i < RB2)) {
                    if(i < RUBIKSIZE) {
                        x = 2;
                        y = i;
                        z = j - RUBIKSIZE;

                        painter.setBrush(CRubik::fromEFace(rubik->getFace(x, y, z, CMouvement::cmedX)));
                    } else if(i < RB2) {
                        if(j < RUBIKSIZE) {
                            x = RB2 - i - 1;
                            y = j;
                            z = 0;

                            painter.setBrush(CRubik::fromEFace(rubik->getFace(x, y, z, CMouvement::cmedY)));
                        } else if(j < RB2) {
                            x = RB2 - i - 1;
                            y = 2;
                            z = j - RUBIKSIZE;

                            painter.setBrush(CRubik::fromEFace(rubik->getFace(x, y, z, CMouvement::cmedZ)));
                        } else if(j < (RB2 + RUBIKSIZE)) {
                            x = RB2 - i - 1;
                            y = RB2 - j + RUBIKSIZE - 1;
                            z = 2;

                            painter.setBrush(CRubik::fromEFace(rubik->getFace(x, y, z, CMouvement::cmedY)));
                        } else {
                            x = RB2 - i - 1;
                            y = 0;
                            z = RB2 + RB2 - j - 1;

                            painter.setBrush(CRubik::fromEFace(rubik->getFace(x, y, z, CMouvement::cmedZ)));
                        }
                    } else {
                        x = 0;
                        y = RB2 - i + RUBIKSIZE - 1;
                        z = j - RUBIKSIZE;

                        painter.setBrush(CRubik::fromEFace(rubik->getFace(x, y, z, CMouvement::cmedX)));
                    }

                    painter.drawRect(j * size + margeX, i * size + margeY, size, size);
                }
            }
        }
    }
}

void CMapView::resizeEvent(QResizeEvent *event) {
    int w = event->size().width();
    int h = event->size().height();
    int sizeX = w / NBX;
    int sizeY = h / NBY;

    if(sizeX > sizeY) {
        margeX = (w - sizeY * NBX) / 2;
        margeY = 0;
        size = sizeY;
    } else {
        margeX = 0;
        margeY = (h - sizeX * NBY) / 2;
        size = sizeX;
    }
}

void CMapView::onUpdate(void) {
    update();
}
