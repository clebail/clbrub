#include "CMainWindow.h"

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
}

void CMainWindow::on_pbX_clicked(bool) {
    w3d->rotate(0, CRubik::crrsX, cbInverse->isChecked());
}

void CMainWindow::on_pbY_clicked(bool) {
    w3d->rotate(3, CRubik::crrsY, cbInverse->isChecked());
}

void CMainWindow::on_pbZ_clicked(bool) {
    w3d->rotate(6, CRubik::crrsZ, cbInverse->isChecked());
}

void CMainWindow::on_pbMelange_clicked(bool) {
    w3d->melange();
}

void CMainWindow::on_pbSolve_clicked(bool) {
    w3d->solve();
}
