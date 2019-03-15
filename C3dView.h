//-----------------------------------------------------------------------------------------------
#ifndef C3DVIEW_H
#define C3DVIEW_H
//-----------------------------------------------------------------------------------------------
#include <QGLWidget>
#include <QOpenGLTexture>
#include <QTimer>
#include "CRubik.h"
//-----------------------------------------------------------------------------------------------
class C3dView : public QGLWidget {
    Q_OBJECT
public:
    explicit C3dView(QWidget *parent = nullptr);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void rotate(int idRotateGroupe, CRubik::ERotate rotateSens);
    void melange(void);
protected:
    virtual void wheelEvent(QWheelEvent * event);
private:
    CRubik rubik;
    float scale;
    QTimer *timer;
    float roty, rotx, rotz;
    QOpenGLTexture *textures[NBFACE];

    void drawRubik(bool forceColor);
private slots:
    void onTimerTimeout(void);
    void onRubikRotateStep(void);
};
//-----------------------------------------------------------------------------------------------
#endif // C3DVIEW_H
//-----------------------------------------------------------------------------------------------
