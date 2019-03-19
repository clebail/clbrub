#include <QtDebug>
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

CRubik::CRubik(const QList<SMouvement>& mouvements) {
    int i;

    init();

    for(i=0;i<mouvements.size();i++) {
        SMouvement mouvement = mouvements.at(i);
        rotate(mouvement.groupe, mouvement.sens, mouvement.inverse, 1, 0);
    }
}

const CRubik::SFace& CRubik::getSubFace(int idCube, int idFace) const {
    return cubes[idCube].faces[idFace];
}

void CRubik::rotate(int idRotateGroupe, CRubik::ERotate rotateSens, bool inverse, int stepCount, unsigned int ts) {
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
                    case CRubik::crrsX:
                        cube->yc = static_cast<int>(zc * coef);
                        cube->zc = static_cast<int>(-yc * coef);
                        break;
                    case CRubik::crrsY:
                        cube->xc = static_cast<int>(zc * coef);
                        cube->zc = static_cast<int>(-xc * coef);
                        break;
                    case CRubik::crrsZ:
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
                        case CRubik::crrsX:
                            cube->faces[j].coords[k][1] = y*c+z*s;
                            cube->faces[j].coords[k][2] = z*c-y*s;
                            break;
                        case CRubik::crrsY:
                            cube->faces[j].coords[k][0] = x*c+z*s;
                            cube->faces[j].coords[k][2] = z*c-x*s;
                            break;
                        case CRubik::crrsZ:
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
            emit(rotatestep());
        }
    }

    calculGroupes();
}

void CRubik::melange(void) {
    srand(static_cast<unsigned int>(time(nullptr)));
    mouvements.clear();

    for(int i=0;i<50;i++) {
        mouvements.append(createMouvement());
    }

    start();
}

int CRubik::getScore(void) const {
    int x, y, z, i, j;
    int score = 0;

    for(z=i=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            for(x=0;x<RUBIKSIZE;x++,i++) {
                for(j=0;j<NBFACE;j++) {
                    const SCube *cube = &cubes[i];
                    int xc = x - MARGIN;
                    int yc = y - MARGIN;
                    int zc = z - MARGIN;

                    score += (xc != cube->xc ? 1 : 0);
                    score += (yc != cube->yc ? 1 : 0);
                    score += (zc != cube->zc ? 1 : 0);
                }
            }
        }
    }

    return score;
}

QList<CRubik::SMouvement> CRubik::solve(void) {
    QList<CRubik::SMouvement> mouvements;
    CRubik *population[TAILLE_POPULATION];
    int i, j;
    bool fini = false;

    //Initialisation de la population
    for(i=0;i<TAILLE_POPULATION;i++) {
        population[i] = new CRubik(this->mouvements);

        for(j=0;j<TAILLE_GENOME;j++) {
            CRubik::SMouvement mouvement = createMouvement();

            population[i]->mouvements.append(mouvement);
            population[i]->rotate(mouvement.groupe, mouvement.sens, mouvement.inverse, 1, 0);
        }
    }

    j = 0;
    while(!fini) {
        //Tri de la population
        triPopulation(population);

        fini = population[0]->getScore() == 0;

        qDebug() << population[0]->getScore() << population[TAILLE_POPULATION-1]->getScore();

        if(!fini) {
            croiseIndividus(population, this->mouvements);
        }

        fini = ++j >= 1000;
    }

    mouvements = population[0]->mouvements;

    for(i=0;i<TAILLE_POPULATION;i++) {
        delete population[i];
    }

    for(int i=0;i<mouvements.size();i++) {
        rotate(mouvements.at(i).groupe, mouvements.at(i).sens, mouvements.at(i).inverse, ROTATE_STEP, 40);
    }

    return mouvements;
}

void CRubik::run(void) {
    for(int i=0;i<mouvements.size();i++) {
        rotate(mouvements.at(i).groupe, mouvements.at(i).sens, mouvements.at(i).inverse, ROTATE_STEP, 20);
    }
}

void CRubik::init(void) {
    int x, y, z, i, j;

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
                    cubes[i].xc = static_cast<int>(fX);
                    cubes[i].yc = static_cast<int>(fY);
                    cubes[i].zc = static_cast<int>(fZ);
                }
            }
        }
    }

    calculGroupes();
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

CRubik::SMouvement CRubik::createMouvement(void) {
    int sens = rand() % 3;
    int face = rand() % RUBIKSIZE;
    CRubik::SMouvement mouvement;

    mouvement.groupe = face + sens * RUBIKSIZE;
    mouvement.sens = static_cast<CRubik::ERotate>(sens);
    mouvement.inverse = rand() % 2 == 1;

    return mouvement;
}

void CRubik::triPopulation(CRubik **population) {
    int i, j;

    for(i=TAILLE_POPULATION-1;i>=1;i--) {
        for(j=0;j<=i-1;j++) {
            if(population[j+1]->getScore() < population[j]->getScore()) {
                swapIndividus(&population[j+1], &population[j]);
            }
        }
    }
}

void CRubik::swapIndividus(CRubik **idv1, CRubik **idv2) {
    CRubik *tmp = *idv1;

    *idv1 = *idv2;
    *idv2 = tmp;
}

void CRubik::croiseIndividus(CRubik **population, const QList<SMouvement> &melange) {
    int i, ir, max;

    i = 1;
    ir = TAILLE_POPULATION - 1;
    max = TAILLE_POPULATION / 2;
    while(i < max) {
        int seuil = rand() % TAILLE_GENOME;

        population[ir]->from(population[i], population[i-1], seuil, melange);
        population[ir-1]->from(population[i-1], population[i], seuil, melange);

        i += 2;
        ir -= 2;
    }
}

void CRubik::from(CRubik *idv1, CRubik *idv2, int seuil, const QList<SMouvement> &melange) {
    CRubik *src = idv1;
    int i;

    init();
    for(i=0;i<melange.size();i++) {
       rotate(melange.at(i).groupe, melange.at(i).sens, melange.at(i).inverse, 1, 0);
    }

    mouvements.clear();
    for(i=0;i<TAILLE_GENOME;i++) {
        if(i == seuil) {
            src = idv2;
        }

        mouvements.append(src->mouvements.at(i));
    }

    if(rand() % 100 > 80) {
        mouvements[rand() % TAILLE_GENOME] = createMouvement();
    }

    for(i=0;i<mouvements.size();i++) {
       rotate(mouvements.at(i).groupe, mouvements.at(i).sens, mouvements.at(i).inverse, 1, 0);
    }
}


CRubik::SMouvement::operator QString(void) const {
    QString result = "";

    result += QString::number(groupe).rightJustified(MVTPAD, '0');
    result += (sens == CRubik::crrsX ? "X" : (sens == CRubik::crrsY ? "Y" : "Z"));
    result += (inverse ? "1" : "0");

    return result;
}

