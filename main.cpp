#include <QApplication>
#include "CMainWindow.h"

CRubik *rubik = new CRubik();

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    CMainWindow w(rubik);

    w.show();

    return a.exec();
}
