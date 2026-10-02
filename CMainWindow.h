#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include <Qsci/qscilexerpython.h>
#include "ui_CMainWindow.h"

class CMainWindow : public QMainWindow, private Ui::CMainWindow {
    Q_OBJECT
public:
    explicit CMainWindow(CRubik *rubik, QWidget *parent = nullptr);
    ~CMainWindow(void);
private:
    CRubik *rubik;
    QsciLexerPython *lexerPY;
    QString scriptPath;

    void runScript(QString script);
    void runMelange(int nb);
private slots:
    void on_pbRun_clicked();
    void on_pbMelange_clicked();
    void on_pbInit_clicked();
    void on_pbSave_clicked();
    void on_pbLoad_clicked();
    void onEnablePbRun(bool enable);
signals:
    void enablePbRun(bool enable);
};

#endif // CMAINWINDOW_H
