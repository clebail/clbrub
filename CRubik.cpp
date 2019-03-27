#include <QtDebug>
#include <QThread>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "CRubik.h"

#define TAILLE_POPULATION                   100
#define TAILLE_GENOME                       25

CRubik::CRubik(void) {
    init();
}

CRubik::CRubik(const QList<CMouvement *>& mouvements) {
    int i;

    init();

    for(i=0;i<mouvements.size();i++) {
        CMouvement *mouvement = mouvements.at(i);

        rotate(mouvement->getGroupe(), mouvement->getSens(), mouvement->getInverse(), 1, 0);
    }
}

CRubik::~CRubik(void) {
    clearMouvements();
}

const CRubik::SFace& CRubik::getSubFace(int idCube, int idFace) const {
    return cubes[idCube].faces[idFace];
}

void CRubik::melange(void) {
    srand(static_cast<unsigned int>(time(nullptr)));

    for(int i=0;i<50;i++) {
        CMouvement *mouvement = CMouvement::createMouvement();

        mouvements.append(mouvement);
        rotate(mouvement->getGroupe(), mouvement->getSens(), mouvement->getInverse(), ROTATE_STEP, 20);
    }
}

int CRubik::getScore(void) const {
    int x, y, z;
    int score = 0;

    for(z=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            for(x=0;x<RUBIKSIZE;x++) {
                score += distance(x, y, z);
            }
        }
    }

    return score;
}

QList<CMouvement *> CRubik::solve(void) {
    QList<CMouvement *> mouvements;

    return mouvements;
}

void CRubik::init(void) {
    int x, y, z, i, j;

    clearMouvements();

    for(z=i=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            for(x=0;x<RUBIKSIZE;x++,i++) {
                float fX = static_cast<float>(x - MARGIN);
                float fY = static_cast<float>(y - MARGIN);
                float fZ = static_cast<float>(z - MARGIN);

                float coords[NBFACE][NBSOMMET][DIMENSION] = {
                    { { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY-UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT } }, //gauche
                    { { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT } }, //droite
                    { { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT } }, //derrière
                    { { fX-UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ+UNIT } }, //devant
                    { { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX-UNIT, fY-UNIT, fZ+UNIT } }, //bas
                    { { fX-UNIT, fY+UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT } }  //haut
                };

                for(j=0;j<NBFACE;j++) {
                    CRubik::EFace face = static_cast<CRubik::EFace>(j);

                    memcpy(&cubes[i].faces[j].coords, &coords[j], sizeof(float[NBSOMMET][DIMENSION]));

                    cubes[i].faces[j].colorFace = CRubik::crefBlack;
                    if((x == 0 && j == 0) || (x == RUBIKSIZE -1 && j == 1)) {
                        cubes[i].faces[j].colorFace = face;
                    }

                    if((z == 0 && j == 2) || (z == RUBIKSIZE -1 && j == 3)) {
                        cubes[i].faces[j].colorFace = face;
                    }

                    if((y == 0 && j == 4) || (y == RUBIKSIZE -1 && j == 5)) {
                        cubes[i].faces[j].colorFace = face;
                    }

                    cubes[i].faces[j].clb = (cubes[i].faces[j].colorFace == CRubik::crefBlanc && x == 1 && y == 2 && z == 1);
                    cubes[i].xc = cubes[i].xo = static_cast<int>(fX);
                    cubes[i].yc = cubes[i].yo = static_cast<int>(fY);
                    cubes[i].zc = cubes[i].zo = static_cast<int>(fZ);
                }
            }
        }
    }

    calculGroupes();
}

void CRubik::exec(QString cmd) {
    QList<CMouvement *> mvts = CMouvement::formString(cmd);
    int i;

    for(i=0;i<mvts.size();i++) {
        CMouvement * mvt = mvts.at(i);

        mouvements.append(mvt);
        rotate(mvt->getGroupe(), mvt->getSens(), mvt->getInverse());
    }
}

int CRubik::distance(int x, int y, int z) const {
    const SCube *cube = findCube(x, y, z);
    int distance = 0;
    int nbChange = 0;

    nbChange += (cube->xc != cube->xo ? 1 : 0);
    nbChange += (cube->yc != cube->yo ? 1 : 0);
    nbChange += (cube->zc != cube->zo ? 1 : 0);

    if(cube->isCoin()) {
        distance = nbChange;
    } else if(cube->isArete()) {
        distance = nbChange <= 2 ? 1 : 2;
    } else {
        distance = 3 - nbChange;
    }

    return distance;
}

void CRubik::calculGroupes(void) {
    int i;
    SCube **groupex;
    SCube **groupey;
    SCube **groupez;

    memset(rGroupes, 0, sizeof(SCube *) * NBFACE * NBCUBEPARFACE);

    for(i=0;i<NBCUBE;i++) {
        int x = cubes[i].xc + MARGIN;
        int y = cubes[i].yc + MARGIN;
        int z = cubes[i].zc + MARGIN;

        groupex = rGroupes[x];
        groupey = rGroupes[y + RUBIKSIZE];
        groupez = rGroupes[z + 2 * RUBIKSIZE];

        groupex[z * RUBIKSIZE + y] = &cubes[i];
        groupey[z * RUBIKSIZE + x] = &cubes[i];
        groupez[y * RUBIKSIZE + x] = &cubes[i];
    }
}

void CRubik::rotate(int idRotateGroupe, CMouvement::ERotate rotateSens, bool inverse, int stepCount, unsigned int ts) {
    int step;
    int coef = (inverse ? -1 : 1);
    double angle = static_cast<double>(90/stepCount) * coef;
    float c = static_cast<float>(cos(angle * M_PI / 180));
    float s = static_cast<float>(sin(angle * M_PI / 180));

    for(step=0;step<stepCount;step++) {
        int i;

        for(i=0;i<NBCUBEPARFACE;i++) {
            if(rGroupes[idRotateGroupe][i] != nullptr) {
                int j;
                SCube *cube = rGroupes[idRotateGroupe][i];

                if(step == 0) {
                    int xc = cube->xc;
                    int yc = cube->yc;
                    int zc = cube->zc;

                    switch(rotateSens) {
                    case CMouvement::crrsX:
                        cube->yc = static_cast<int>(zc * coef);
                        cube->zc = static_cast<int>(-yc * coef);
                        break;
                    case CMouvement::crrsY:
                        cube->xc = static_cast<int>(zc * coef);
                        cube->zc = static_cast<int>(-xc * coef);
                        break;
                    case CMouvement::crrsZ:
                        cube->xc = static_cast<int>(-yc * coef);
                        cube->yc = static_cast<int>(xc * coef);
                        break;
                    }
                }

                for(j=0;j<NBFACE;j++) {
                    int k;

                    for(k=0;k<NBSOMMET;k++) {

                        float x = cube->faces[j].coords[k][0];
                        float y = cube->faces[j].coords[k][1];
                        float z = cube->faces[j].coords[k][2];

                        switch(rotateSens) {
                        case CMouvement::crrsX:
                            cube->faces[j].coords[k][1] = y*c+z*s;
                            cube->faces[j].coords[k][2] = z*c-y*s;
                            break;
                        case CMouvement::crrsY:
                            cube->faces[j].coords[k][0] = x*c+z*s;
                            cube->faces[j].coords[k][2] = z*c-x*s;
                            break;
                        case CMouvement::crrsZ:
                            cube->faces[j].coords[k][0] = x*c-y*s;
                            cube->faces[j].coords[k][1] = y*c+x*s;
                            break;
                        }
                    }
                }
            }
        }

        if(ts != 0) {
            QThread::currentThread()->msleep(ts);
            emit(update());
        }
    }

    calculGroupes();
}

void CRubik::clearMouvements(void) {
    for(int i=0;i<mouvements.size();i++) {
        delete mouvements.at(i);
    }

    mouvements.clear();
}

CRubik::SCube * CRubik::findCube(int x, int y, int z) const {
    int i;

    for(i=0;i<NBCUBE;i++) {
        if(cubes[i].xc == x - MARGIN && cubes[i].yc == y - MARGIN && cubes[i].zc == z - MARGIN) {
            return const_cast<CRubik::SCube *>(&cubes[i]);
        }
    }

    return nullptr;
}


