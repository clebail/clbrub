#include <QPainter>
#include "CMapView.h"

CMapView::CMapView(QWidget *parent) : QWidget(parent) {
    rubik = nullptr;
}

void CMapView::setRubik(CRubik *rubik) {
    this->rubik = rubik;
    if(rubik != nullptr) {
        connect(rubik, SIGNAL(update()), this, SLOT(onUpdate()));
        update();
    }
}

void CMapView::paintEvent(QPaintEvent *) {
    QPainter painter(this);
}

void CMapView::resizeEvent(QResizeEvent *) {

}

void CMapView::onUpdate(void) {
    update();
}
