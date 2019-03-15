#ifndef CRUBIK_H
#define CRUBIK_H

#include <QObject>
#include <QColor>

#define DIMENSION                   3
#define NBSOMMET                    4
#define NBFACE                      6
#define RUBIKSIZE                   3
#define MARGIN                      (RUBIKSIZE / 2)
#define NBCUBE                      (RUBIKSIZE * RUBIKSIZE * RUBIKSIZE)
#define NBCUBEPARFACE               (RUBIKSIZE * RUBIKSIZE)
#define UNIT                        0.5f
#define NBROTATEGROUPE              (NBFACE + (RUBIKSIZE - 2) * DIMENSION)

class CRubik : public QObject {
    Q_OBJECT
public:
    typedef enum { crefRouge, crefOrange, crefBlue, crefVert, crefJaune, crefBlanc } EFace;
    typedef enum { crrsX, crrsY, crrsZ } ERotate;

    typedef struct _SFace {
        float coords[NBSOMMET][DIMENSION];
        QColor color;
        CRubik::EFace colorFace;
    }SFace;

    CRubik(void);

    const CRubik::SFace& getSubFace(int idCube, int idFace) const;
    void rotate(int idRotateGroupe, CRubik::ERotate rotateSens, int ts = 40000);
    void melange(void);
private:
    typedef struct _SCube {
        SFace faces[NBFACE];
        int xc, yc, zc;
    }SCube;

    SCube cubes[NBCUBE];
    SCube *rGroupes[NBROTATEGROUPE][NBCUBEPARFACE];

    void init(void);
    void calculGroupes(void);
signals:
    void rotatestep(void);
};

#endif // CRUBIK_H
