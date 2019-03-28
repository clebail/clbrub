#include <Python.h>
#include <QtConcurrent>
#include "CMainWindow.h"

CMainWindow::CMainWindow(CRubik *rubik, QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
    this->rubik = rubik;

    teScript->setText("import rubik\nrubik.melange(50)");

    w3d->setRubik(rubik);

    connect(this, SIGNAL(enablePbRun(bool)), this, SLOT(onEnablePbRun(bool)));
}

void CMainWindow::runScript(const char* script) {
    emit(enablePbRun(false));
    PyRun_SimpleString(script);
    emit(enablePbRun(true));
}

void CMainWindow::on_pbRun_clicked() {
    char *script = teScript->toPlainText().toLatin1().data();

    QtConcurrent::run(this, &CMainWindow::runScript, script);
}

void CMainWindow::onEnablePbRun(bool enable) {
     pbRun->setEnabled(enable);
}
