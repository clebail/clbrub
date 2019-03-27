#ifndef CRUBIK_H
#define CRUBIK_H

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
    Q_OBJECT
public:
    typedef enum { crefRouge, crefOrange, crefBlue, crefVert, crefJaune, crefBlanc, crefBlancClb, crefBlack } EFace;

    typedef struct _SFace {
        float coords[NBSOMMET][DIMENSION];
        int clb;
        CRubik::EFace colorFace;
    }SFace;

    CRubik(void);
    CRubik(const QList<CMouvement *>& mouvements);
    ~CRubik(void);

    const CRubik::SFace& getSubFace(int idCube, int idFace) const;
    void melange(void);
    int getScore(void) const;
    QList<CMouvement *> solve(void);
    void init(void);
    void exec(QString cmd);
private:
    typedef struct _SCube {
        SFace faces[NBFACE];
        int xc, yc, zc;
    }SCube;

    SCube cubes[NBCUBE];
    SCube *rGroupes[NBROTATEGROUPE][NBCUBEPARFACE];
    QList<CMouvement *> mouvements;

    void calculGroupes(void);
    void rotate(int idRotateGroupe, CMouvement::ERotate rotateSens, bool inverse, int stepCount = ROTATE_STEP, unsigned int ts = 40);
    void clearMouvements(void);
signals:
    void update(void);
};

#endif // CRUBIK_H
