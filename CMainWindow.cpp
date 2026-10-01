#include <Python.h>
#include <QtConcurrent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSaveFile>
#include <string>
#include <string.h>
#include "CMainWindow.h"

CMainWindow::CMainWindow(CRubik *rubik, QWidget *parent) : QMainWindow(parent) {
    setupUi(this);
    this->rubik = rubik;

    teScript->setText("import rubik\nrubik.melange(50, True)");
    teScript->setModified(false);

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
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyRun_SimpleString(buffer);
    PyGILState_Release(gstate);
    emit(enablePbRun(true));

    delete[] buffer;
}

void CMainWindow::on_pbRun_clicked() {
    (void)QtConcurrent::run(&CMainWindow::runScript, this, teScript->text());
}

void CMainWindow::on_pbSave_clicked() {
    QString path = QFileDialog::getSaveFileName(this, "Sauver le script", scriptPath, "Scripts Python (*.py);;Tous les fichiers (*)");
    if(path.isEmpty()) {
        return;
    }
    if(QFileInfo(path).suffix().isEmpty()) {
        path += ".py";
    }

    // QSaveFile : écriture dans un fichier temporaire puis remplacement, l'ancien script reste intact en cas d'échec
    QSaveFile file(path);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text) || file.write(teScript->text().toUtf8()) < 0 || !file.commit()) {
        QMessageBox::warning(this, "Sauver le script", "Impossible d'écrire " + path + " : " + file.errorString());
        return;
    }
    teScript->setModified(false);
    scriptPath = path;
}

void CMainWindow::on_pbLoad_clicked() {
    if(teScript->isModified()) {
        QMessageBox::StandardButton rep = QMessageBox::question(this, "Charger un script", "Le script en cours a été modifié et n'est pas sauvé. Le remplacer quand même ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(rep != QMessageBox::Yes) {
            return;
        }
    }

    QString path = QFileDialog::getOpenFileName(this, "Charger un script", scriptPath, "Scripts Python (*.py);;Tous les fichiers (*)");
    if(path.isEmpty()) {
        return;
    }

    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Charger un script", "Impossible de lire " + path + " : " + file.errorString());
        return;
    }
    teScript->setText(QString::fromUtf8(file.readAll()));
    teScript->setModified(false);
    scriptPath = path;
}

void CMainWindow::onEnablePbRun(bool enable) {
     pbRun->setEnabled(enable);
}
