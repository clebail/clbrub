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

void CRubik::melange(int nb, bool anim) {
    for(int i=0;i<nb;i++) {
        CMouvement *mouvement = CMouvement::createMouvement();

        mouvements.append(mouvement);
        if(anim) {
            rotate(mouvement->getGroupe(), mouvement->getSens(), mouvement->getInverse(), ROTATE_STEP, 20);
        } else {
            rotate(mouvement->getGroupe(), mouvement->getSens(), mouvement->getInverse(), 1, 0);
        }
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

    //clearMouvements();

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

                cubes[i].faces[0].origineOrientation = cubes[i].faces[1].origineOrientation = CMouvement::cmedX;
                cubes[i].faces[2].origineOrientation = cubes[i].faces[3].origineOrientation = CMouvement::cmedY;
                cubes[i].faces[4].origineOrientation = cubes[i].faces[5].origineOrientation = CMouvement::cmedZ;

                cubes[i].faces[0].orientation = cubes[i].faces[1].orientation = CMouvement::cmedX;
                cubes[i].faces[2].orientation = cubes[i].faces[3].orientation = CMouvement::cmedY;
                cubes[i].faces[4].orientation = cubes[i].faces[5].orientation = CMouvement::cmedZ;

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
    if(x != 1 || y != 1 || z != 1) {
        int nbChange = 0;

        nbChange += (cube->xc != cube->xo ? 1 : 0);
        nbChange += (cube->yc != cube->yo ? 1 : 0);
        nbChange += (cube->zc != cube->zo ? 1 : 0);

        if(cube->isCoin()) {
            distance = nbChange;
            if(!cube->isOriented()) {
                distance += 3;
            }
        } else if(cube->isArete()) {
            distance = nbChange <= 2 ? 1 : 2;
            if(!cube->isOriented()) {
                distance += 3;
            }
        } else {
            distance = 3 - nbChange;
        }
    }

    return distance;
}

void CRubik::printCubeInfo(int x, int y, int z) const {
    const SCube *cube = findCube(x, y, z);
    int i;

    qDebug() << "Position originale (" << cube->xo << "," << cube->yo << "," << cube->zo << ")";
    qDebug() << "Position actuelle (" << cube->xc << "," << cube->yc << "," << cube->zc << ")";
    qDebug() << "Orientation des faces";

    for(i=0;i<NBFACE;i++) {
        const SFace *face = &cube->faces[i];

        if(face->colorFace != CRubik::crefBlack) {
            QString faceNames[] = { "Rouge", "Orange", "Blue", "Vert", "Jaune", "Blanc", "Blanc" };
            QString orientationNames[] = { "X" , "Y", "Z" };

            qDebug() << "Couleur" << faceNames[face->colorFace] << "Orientation originale" << orientationNames[face->origineOrientation] << "Orientation actuelle" << orientationNames[face->orientation];
        }
    }
}

QString CRubik::getLastMouvement(void) const {
    if(mouvements.size() != 0) {
        CMouvement *mvt = mouvements.last();

        return *mvt;
    }

    return "";
}

CRubik::EFace  CRubik::getFace(int x, int y, int z, CMouvement::EDirection direction) {
    SCube *cube = findCube(x, y, z);
    int i;

    for(i=0;i<NBFACE;i++) {
        SFace *face = &cube->faces[i];

        if(face->orientation == direction && face->colorFace != CRubik::crefBlack) {
            return face->colorFace == CRubik::crefBlancClb ? CRubik::crefBlanc : face->colorFace;
        }
    }

    return CRubik::crefBlack;
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

void CRubik::rotate(int idRotateGroupe, CMouvement::EDirection rotateSens, bool inverse, int stepCount, unsigned int ts) {
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
                    case CMouvement::cmedX:
                        cube->yc = static_cast<int>(zc * coef);
                        cube->zc = static_cast<int>(-yc * coef);
                        break;
                    case CMouvement::cmedY:
                        cube->xc = static_cast<int>(zc * coef);
                        cube->zc = static_cast<int>(-xc * coef);
                        break;
                    case CMouvement::cmedZ:
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
                        case CMouvement::cmedX:
                            cube->faces[j].coords[k][1] = y*c+z*s;
                            cube->faces[j].coords[k][2] = z*c-y*s;
                            break;
                        case CMouvement::cmedY:
                            cube->faces[j].coords[k][0] = x*c+z*s;
                            cube->faces[j].coords[k][2] = z*c-x*s;
                            break;
                        case CMouvement::cmedZ:
                            cube->faces[j].coords[k][0] = x*c-y*s;
                            cube->faces[j].coords[k][1] = y*c+x*s;
                            break;
                        }
                    }

                    if(step == 0) {
                        switch(rotateSens) {
                        case CMouvement::cmedX:
                            if(cube->faces[j].orientation == CMouvement::cmedY) {
                                cube->faces[j].orientation = CMouvement::cmedZ;
                            } else if(cube->faces[j].orientation == CMouvement::cmedZ) {
                                cube->faces[j].orientation = CMouvement::cmedY;
                            }
                            break;
                        case CMouvement::cmedY:
                            if(cube->faces[j].orientation == CMouvement::cmedX) {
                                cube->faces[j].orientation = CMouvement::cmedZ;
                            } else if(cube->faces[j].orientation == CMouvement::cmedZ) {
                                cube->faces[j].orientation = CMouvement::cmedX;
                            }
                            break;
                        case CMouvement::cmedZ:
                            if(cube->faces[j].orientation == CMouvement::cmedX) {
                                cube->faces[j].orientation = CMouvement::cmedY;
                            } else if(cube->faces[j].orientation == CMouvement::cmedY) {
                                cube->faces[j].orientation = CMouvement::cmedX;
                            }
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
    while(mouvements.size()) {
        delete mouvements.takeFirst();
    }
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


