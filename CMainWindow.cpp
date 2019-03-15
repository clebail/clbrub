#include "CMainWindow.h"

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
}

void CMainWindow::on_pbX_clicked(bool) {
    w3d->rotate(0, CRubik::crrsX);
}

void CMainWindow::on_pbY_clicked(bool) {
    w3d->rotate(3, CRubik::crrsY);
}

void CMainWindow::on_pbZ_clicked(bool) {
    w3d->rotate(6, CRubik::crrsZ);
}

void CMainWindow::on_pbMelange_clicked(bool) {
    w3d->melange();
}
