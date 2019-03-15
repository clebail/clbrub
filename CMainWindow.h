#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include "ui_CMainWindow.h"

class CMainWindow : public QMainWindow, private Ui::CMainWindow {
    Q_OBJECT
public:
    explicit CMainWindow(QWidget *parent = nullptr);
private slots:
    void on_pbX_clicked(bool checked=false);
    void on_pbY_clicked(bool checked=false);
    void on_pbZ_clicked(bool checked=false);
    void on_pbMelange_clicked(bool checked=false);
};

#endif // CMAINWINDOW_H
