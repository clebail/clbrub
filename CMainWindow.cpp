#include "CMainWindow.h"

CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
}

void CMainWindow::on_pbX_clicked(bool) {
    w3d->rotate(0, CMouvement::crrsX, cbInverse->isChecked());
}

void CMainWindow::on_pbY_clicked(bool) {
    w3d->rotate(3, CMouvement::crrsY, cbInverse->isChecked());
}

void CMainWindow::on_pbZ_clicked(bool) {
    w3d->rotate(6, CMouvement::crrsZ, cbInverse->isChecked());
}

void CMainWindow::on_pbMelange_clicked(bool) {
    w3d->melange();
}

void CMainWindow::on_pbSolve_clicked(bool) {
    w3d->solve();
}
