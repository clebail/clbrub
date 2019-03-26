#include "CMainWindow.h"

CMainWindow::CMainWindow(CRubik *rubik, QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
    this->rubik = rubik;

    w3d->setRubik(rubik);
}

void CMainWindow::on_pbRun_clicked() {
    rubik->melange(w3d);
}
