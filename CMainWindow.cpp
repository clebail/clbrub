#include <Python.h>
#include <QtConcurrent>
#include <string>
#include <string.h>
#include "CMainWindow.h"

CMainWindow::CMainWindow(CRubik *rubik, QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
    this->rubik = rubik;

    teScript->setText("import rubik\nrubik.melange(50, True)");

    lexerPY = new QsciLexerPython();
    teScript->setLexer(lexerPY);
    teScript->setMarginLineNumbers(1, true);
    teScript->setMarginWidth(1, 30);
    teScript->setMarginsFont(QFont("Hack", 8));

    w3d->setRubik(rubik);
    wMap->setRubik(rubik);

    connect(this, SIGNAL(enablePbRun(bool)), this, SLOT(onEnablePbRun(bool)));
}

CMainWindow::~CMainWindow(void) {
    delete lexerPY;
}

void CMainWindow::runScript(QString script) {
    std::string stdScript = script.toStdString();
    char *buffer = new char[stdScript.size()+1];
    strcpy(buffer, stdScript.c_str());

    emit(enablePbRun(false));
    PyRun_SimpleString(buffer);
    emit(enablePbRun(true));
}

void CMainWindow::on_pbRun_clicked() {
    QtConcurrent::run(this, &CMainWindow::runScript, teScript->text());
}

void CMainWindow::onEnablePbRun(bool enable) {
     pbRun->setEnabled(enable);
}
