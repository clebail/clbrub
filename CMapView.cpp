#include <QtDebug>
#include <QPainter>
#include <QResizeEvent>
#include "CMapView.h"

#define NBX             (RUBIKSIZE * 4)
#define NBY             (RUBIKSIZE * 3)

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
    // Patron standard : U au-dessus de F, L F R B côte à côte, D en dessous (colonne, ligne du coin haut gauche, en faces)
    static const int patron[NBFACE][2] = { { 1, 0 }, { 2, 1 }, { 1, 1 }, { 1, 2 }, { 0, 1 }, { 3, 1 } };   // U R F D L B
    QPainter painter(this);
    int f, r, c;

    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    painter.drawRect(geometry());

    if(rubik != nullptr) {
        // Les autocollants de CCubeCore sont rangés dans l'ordre du patron : face, puis ligne, puis colonne
        for(f=0;f<NBFACE;f++) {
            for(r=0;r<RUBIKSIZE;r++) {
                for(c=0;c<RUBIKSIZE;c++) {
                    int i = patron[f][1] * RUBIKSIZE + r;
                    int j = patron[f][0] * RUBIKSIZE + c;

                    painter.setBrush(CRubik::fromEFace(rubik->getStickerColor((f * RUBIKSIZE + r) * RUBIKSIZE + c)));
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
