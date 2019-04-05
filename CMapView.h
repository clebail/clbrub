#ifndef CMAPVIEW_H
#define CMAPVIEW_H

#include <QWidget>
#include "CRubik.h"

class CMapView : public QWidget {
    Q_OBJECT
public:
    explicit CMapView(QWidget *parent = nullptr);
    void setRubik(CRubik *rubik);
protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
private:
    CRubik *rubik;
    int margeX, margeY, size;
private slots:
    void onUpdate(void);
};

#endif // CMAPVIEW_H
