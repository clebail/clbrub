//-----------------------------------------------------------------------------------------------
#include <QtDebug>
#include <QWheelEvent>
#include <QMouseEvent>
#include "C3dView.h"
//-----------------------------------------------------------------------------------------------
C3dView::C3dView(QWidget *parent) : QGLWidget(parent) {
    scale = 1.0;
    rotx = roty = rotz = 0.0;
    roty = 45.0;
    rotx = 30.0;
    timer = new QTimer(this);

    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    timer->setInterval(400);
    //timer->start();

    memset(textures, 0, sizeof(GLuint) * (NBFACE + 1));

    rubik = nullptr;
}
//-----------------------------------------------------------------------------------------------
C3dView::~C3dView(void) {
}
//-----------------------------------------------------------------------------------------------
void C3dView::initializeGL() {
    qglClearColor(Qt::black);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    loadTexture(":/textures/bleu.png", &textures[CRubik::crefBlue]);
    loadTexture(":/textures/vert.png", &textures[CRubik::crefVert]);
    loadTexture(":/textures/blanc.png", &textures[CRubik::crefBlanc]);
    loadTexture(":/textures/jaune.png", &textures[CRubik::crefJaune]);
    loadTexture(":/textures/rouge.png", &textures[CRubik::crefRouge]);
    loadTexture(":/textures/orange.png", &textures[CRubik::crefOrange]);
    loadTexture(":/textures/blancClb.png", &textures[NBFACE]);
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
void C3dView::setRubik(CRubik *rubik) {
    this->rubik = rubik;
    if(rubik != nullptr) {
        connect(rubik, SIGNAL(update()), this, SLOT(updateGL()));
        updateGL();
    }
}
//-----------------------------------------------------------------------------------------------
void C3dView::wheelEvent(QWheelEvent * event) {
    event->delta() > 0 ? scale += scale*0.1f : scale -= scale*0.1f;
    updateGL();
}
//-----------------------------------------------------------------------------------------------
void C3dView::mousePressEvent(QMouseEvent * event) {
    lastPos = event->pos();
}
//-----------------------------------------------------------------------------------------------
void C3dView::mouseMoveEvent(QMouseEvent * event) {
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotx += 4 * dy;
        roty += 4 * dx;
    } else if (event->buttons() & Qt::RightButton) {
        rotx += 4 * dy;
        rotz += 4 * dx;
    }

    updateGL();

    lastPos = event->pos();
}
//-----------------------------------------------------------------------------------------------
void C3dView::drawRubik(bool forceColor) {
    int x, y, z, i, j;

    if(rubik != nullptr) {
        for(z=i=0;z<RUBIKSIZE;z++) {
            for(y=0;y<RUBIKSIZE;y++) {
                for(x=0;x<RUBIKSIZE;x++,i++) {
                    for(j=0;j<NBFACE;j++) {
                        CRubik::SFace face = rubik->getSubFace(i, j);

                        if(forceColor && face.colorFace == CRubik::crefBlack) {
                            qglColor(Qt::black);
                        } else {
                            GLuint texture = textures[face.colorFace];
                            glEnable(GL_TEXTURE_2D);

                            if(face.clb) {
                                texture = textures[NBFACE];
                            }
                            glBindTexture(GL_TEXTURE_2D, texture);
                        }

                        glBegin(GL_QUADS);

                        glTexCoord2d(0, 1);
                        glVertex3f(face.coords[0][0], face.coords[0][1], face.coords[0][2]);
                        glTexCoord2d(0, 0);
                        glVertex3f(face.coords[1][0], face.coords[1][1], face.coords[1][2]);
                        glTexCoord2d(1, 0);
                        glVertex3f(face.coords[2][0], face.coords[2][1], face.coords[2][2]);
                        glTexCoord2d(1, 1);
                        glVertex3f(face.coords[3][0], face.coords[3][1], face.coords[3][2]);

                        glEnd();
                        glDisable(GL_TEXTURE_2D);
                    }
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------------------------
void C3dView::loadTexture(QString textureName, GLuint *texture) {
    QImage im(textureName);
    QImage tex = QGLWidget::convertToGLFormat(im);

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glDisable(GL_TEXTURE_2D);
}
//-----------------------------------------------------------------------------------------------
void C3dView::onTimerTimeout(void) {
    rotx=static_cast<float>((static_cast<int>(rotx) + 5) % 360);
    //roty=static_cast<float>((static_cast<int>(roty) + 5) % 360);
    //rotz=static_cast<float>((static_cast<int>(rotz) + 5) % 360);

    updateGL();
}
//-----------------------------------------------------------------------------------------------

