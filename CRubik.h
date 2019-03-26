#ifndef CRUBIK_H
#define CRUBIK_H

#include <QThread>
#include <QColor>
#include <QList>
#include <QGLWidget>
#include "CMouvement.h"

#define DIMENSION                   3
#define NBSOMMET                    4
#define NBFACE                      6
#define MARGIN                      (RUBIKSIZE / 2)
#define NBCUBE                      (RUBIKSIZE * RUBIKSIZE * RUBIKSIZE)
#define NBCUBEPARFACE               (RUBIKSIZE * RUBIKSIZE)
#define UNIT                        0.5f
#define NBROTATEGROUPE              (NBFACE + (RUBIKSIZE - 2) * DIMENSION)
#define ROTATE_STEP                 5

class CRubik : public QObject {
public:
    typedef enum { crefRouge, crefOrange, crefBlue, crefVert, crefJaune, crefBlanc, crefBlancClb, crefBlack } EFace;

    typedef struct _SFace {
        float coords[NBSOMMET][DIMENSION];
        int clb;
        CRubik::EFace colorFace;
    }SFace;

    CRubik(void);
    CRubik(const QList<CMouvement>& mouvements);

    const CRubik::SFace& getSubFace(int idCube, int idFace) const;
    void rotate(int idRotateGroupe, CMouvement::ERotate rotateSens, bool inverse, int stepCount = ROTATE_STEP, unsigned int ts = 40, QGLWidget *render = nullptr);
    void melange(QGLWidget *render = nullptr);
    int getScore(void) const;
    QList<CMouvement> solve(void);
private:
    typedef struct _SCube {
        SFace faces[NBFACE];
        int xc, yc, zc;
    }SCube;

    SCube cubes[NBCUBE];
    SCube *rGroupes[NBROTATEGROUPE][NBCUBEPARFACE];
    QList<CMouvement> mouvements;

    void init(void);
    void calculGroupes(void);
    CMouvement createMouvement(void);
};

#endif // CRUBIK_H
