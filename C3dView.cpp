//-----------------------------------------------------------------------------------------------
#include <QtDebug>
#include <QWheelEvent>
#include "C3dView.h"
//-----------------------------------------------------------------------------------------------
C3dView::C3dView(QWidget *parent) : QGLWidget(parent), rubik() {
    scale = 1.0;
    rotx = roty = rotz = 0.0;
    roty = 45.0;
    rotx = 30.0;
    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    timer->setInterval(40);
    //timer->start();

    memset(textures, 0, sizeof(textures));

    connect(&rubik, SIGNAL(rotatestep()), this, SLOT(onRubikRotateStep()));
}
//-----------------------------------------------------------------------------------------------
void C3dView::initializeGL() {
    qglClearColor(Qt::black);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    textures[CRubik::crefOrange] = new QOpenGLTexture(QImage(":/textures/orange.png"));
}
//-----------------------------------------------------------------------------------------------
void C3dView::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLdouble x = static_cast<GLdouble>(width / height);
    glFrustum(-x, x, -1.0, 1.0, 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}
//-----------------------------------------------------------------------------------------------
void C3dView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -11.0);

    glRotatef(rotx, 1.0, 0.0, 0.0);
    glRotatef(roty, 0.0, 1.0, 0.0);
    glRotatef(rotz, 0.0, 0.0, 1.0);

    glScalef(scale, scale, scale);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    drawRubik(true);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    drawRubik(false);
}
//-----------------------------------------------------------------------------------------------
void C3dView::rotate(int idRotateGroupe, CRubik::ERotate rotateSens) {
    rubik.rotate(idRotateGroupe, rotateSens);
}
//-----------------------------------------------------------------------------------------------
void C3dView::melange(void) {
    rubik.melange();
}
//-----------------------------------------------------------------------------------------------
void C3dView::wheelEvent(QWheelEvent * event) {
    event->delta() > 0 ? scale += scale*0.1f : scale -= scale*0.1f;
    updateGL();
}
//-----------------------------------------------------------------------------------------------
void C3dView::drawRubik(bool forceColor) {
    int x, y, z, i, j, k;

    for(z=i=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            for(x=0;x<RUBIKSIZE;x++,i++) {
                for(j=0;j<NBFACE;j++) {
                    CRubik::SFace face = rubik.getSubFace(i, j);


                    glBegin(GL_QUADS);
                    qglColor(forceColor ? Qt::black : face.color);
                    for(k=0;k<NBSOMMET;++k) {
                        glVertex3f(face.coords[k][0], face.coords[k][1], face.coords[k][2]);
                    }
                    glEnd();
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------------------------
void C3dView::onTimerTimeout(void) {
    rotx=static_cast<float>((static_cast<int>(rotx) + 5) % 360);
    roty=static_cast<float>((static_cast<int>(roty) + 5) % 360);
    rotz=static_cast<float>((static_cast<int>(rotz) + 5) % 360);

    updateGL();
}
//-----------------------------------------------------------------------------------------------
void C3dView::onRubikRotateStep(void) {
    updateGL();
}
//-----------------------------------------------------------------------------------------------
