#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include "ui_CMainWindow.h"

class CMainWindow : public QMainWindow, private Ui::CMainWindow {
    Q_OBJECT
public:
    explicit CMainWindow(CRubik *rubik, QWidget *parent = nullptr);
private:
    CRubik *rubik;

    void runScript(const char* script);
private slots:
    void on_pbRun_clicked();
};

#endif // CMAINWINDOW_H
