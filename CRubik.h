#ifndef CRUBIK_H
#define CRUBIK_H

#include <QColor>
#include <QList>
#include <QGLWidget>
#include "CMouvement.h"

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
        CMouvement::EDirection orientation;
        CMouvement::EDirection origineOrientation;
    }SFace;

    CRubik(void);

    const CRubik::SFace& getSubFace(int idCube, int idFace) const;
    QString melange(int nb = 50, bool anim = true);
    void init(void);
    QString exec(QString cmd);
    void printCubeInfo(int x, int y, int z) const;
    CRubik::EFace getFace(int x, int y, int z, CMouvement::EDirection direction) const;
    bool win(void);
    static QColor fromEFace(CRubik::EFace colorFace);
private:
    typedef struct _SCube {
        SFace faces[NBFACE];
        int xc, yc, zc;
        int xo, yo, zo;

        bool isCoin(void) const {
            return (abs(xc) + abs(yc) + abs(zc)) == 3;
        }

        bool isArete(void) const  {
            return (abs(xc) + abs(yc) + abs(zc)) == 2;
        }

        bool isCentre(void) const  {
            return !isCoin() && !isArete();
        }

        bool isOriented(void) const {
            for(int i=0;i<NBFACE;i++) {
                if(faces[i].orientation != faces[i].origineOrientation) {
                    return false;
                }
            }

            return true;
        }

        operator QString(void) const {
            return "("+QString::number(xc)+", "+QString::number(yc)+", "+QString::number(zc)+") - ("+QString::number(xo)+", "+QString::number(yo)+", "+QString::number(zo)+")";
        }
    }SCube;

    SCube cubes[NBCUBE];
    SCube *rGroupes[NBROTATEGROUPE][NBCUBEPARFACE];

    void calculGroupes(void);
    void rotate(int idRotateGroupe, CMouvement::EDirection rotateSens, bool inverse, int stepCount = ROTATE_STEP, unsigned int ts = 40);
    SCube * findCube(int x, int y, int z) const;
signals:
    void update(void);
    void endRotate(void);
};

#endif // CRUBIK_H
