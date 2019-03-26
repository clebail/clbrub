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
    ~C3dView(void);
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
    void rotate(int idRotateGroupe, CMouvement::ERotate rotateSens, bool inverse=false);
    void melange(void);
    void solve(void);
protected:
    virtual void wheelEvent(QWheelEvent * event);
private:
    CRubik *rubik;
    float scale;
    QTimer *timer;
    float roty, rotx, rotz;
    GLuint textures[NBFACE+1];

    void drawRubik(bool forceColor);
    void loadTexture( QString textureName, GLuint *texture);
    void renderRotate(void);
private slots:
    void onTimerTimeout(void);
};
//-----------------------------------------------------------------------------------------------
#endif // C3DVIEW_H
//-----------------------------------------------------------------------------------------------
