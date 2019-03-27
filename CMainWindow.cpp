#include <Python.h>
#include <QtConcurrent>
#include "CMainWindow.h"

CMainWindow::CMainWindow(CRubik *rubik, QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
    this->rubik = rubik;

    teScript->setText("import rubik\nrubik.melange()");

    w3d->setRubik(rubik);
}

void CMainWindow::runScript(const char* script) {
   PyRun_SimpleString(script);
}

void CMainWindow::on_pbRun_clicked() {
    QFuture<void> f = QtConcurrent::run(this, &CMainWindow::runScript, teScript->toPlainText().toLatin1().data());
}
