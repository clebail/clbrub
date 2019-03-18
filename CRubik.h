#ifndef CRUBIK_H
#define CRUBIK_H

#include <QThread>
#include <QColor>
#include <QList>

#define DIMENSION                   3
#define NBSOMMET                    4
#define NBFACE                      6
#define RUBIKSIZE                   3
#define MARGIN                      (RUBIKSIZE / 2)
#define NBCUBE                      (RUBIKSIZE * RUBIKSIZE * RUBIKSIZE)
#define NBCUBEPARFACE               (RUBIKSIZE * RUBIKSIZE)
#define UNIT                        0.5f
#define NBROTATEGROUPE              (NBFACE + (RUBIKSIZE - 2) * DIMENSION)
#define ROTATE_STEP                 5
#define MVTPAD                      3

class CRubik : public QThread {
    Q_OBJECT
public:
    typedef enum { crefRouge, crefOrange, crefBlue, crefVert, crefJaune, crefBlanc, crefBlancClb, crefBlack } EFace;
    typedef enum { crrsX, crrsY, crrsZ } ERotate;

    typedef struct _SFace {
        float coords[NBSOMMET][DIMENSION];
        int clb;
        CRubik::EFace colorFace;
    }SFace;

    typedef struct _SMouvement {
        int groupe;
        CRubik::ERotate sens;
        bool inverse;

        _SMouvement(void) {}

        _SMouvement(QString source) {
            QString sSens = source.mid(3, 1);
            groupe = source.mid(0, MVTPAD).toInt();
            sens = (sSens == "X" ? CRubik::crrsX : (sSens == "Y" ? CRubik::crrsY : CRubik::crrsZ));
            inverse = source.mid(4, 1).toInt() == 1;
        }

        operator QString(void) const;
    }SMouvement;

    CRubik(void);
    CRubik(const QList<SMouvement>& mouvements);

    const CRubik::SFace& getSubFace(int idCube, int idFace) const;
    void rotate(int idRotateGroupe, CRubik::ERotate rotateSens, bool inverse, int stepCount = ROTATE_STEP, unsigned int ts = 40);
    void melange(void);
    int getScore(void) const;
    QList<CRubik::SMouvement> solve(void);
protected:
    void run(void);
private:
    typedef struct _SCube {
        SFace faces[NBFACE];
        int xc, yc, zc;
    }SCube;

    SCube cubes[NBCUBE];
    SCube *rGroupes[NBROTATEGROUPE][NBCUBEPARFACE];
    QList<CRubik::SMouvement> mouvements;

    void init(void);
    void calculGroupes(void);
    CRubik::SMouvement createMouvement(void);
    void triPopulation(CRubik **population);
    void swapIndividus(CRubik **idv1, CRubik **idv2);
    void croiseIndividus(CRubik **population);
    void from(CRubik *idv1, CRubik *idv2, int seuil);
signals:
    void rotatestep(void);
};

#endif // CRUBIK_H
