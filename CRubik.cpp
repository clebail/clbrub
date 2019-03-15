#include <QtDebug>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "CRubik.h"

#define ROTATE_STEP             5
#define ANGLE                   static_cast<double>(90/ROTATE_STEP)

CRubik::CRubik(void) {
    init();
}

const CRubik::SFace& CRubik::getSubFace(int idCube, int idFace) const {
    return cubes[idCube].faces[idFace];
}

void CRubik::rotate(int idRotateGroupe, ERotate rotateSens, int ts) {
    int step;
    float c = static_cast<float>(cos(ANGLE * M_PI / 180));
    float s = static_cast<float>(sin(ANGLE * M_PI / 180));

    for(step=0;step<ROTATE_STEP;step++) {
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
                        cube->yc = static_cast<int>(zc);
                        cube->zc = static_cast<int>(-yc);
                        break;
                    case CRubik::crrsY:
                        cube->xc = static_cast<int>(zc);
                        cube->zc = static_cast<int>(-xc);
                        break;
                    case CRubik::crrsZ:
                        cube->xc = static_cast<int>(-yc);
                        cube->yc = static_cast<int>(xc);
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

        usleep(ts);
        emit(rotatestep());
    }

    calculGroupes();
}

void CRubik::melange(void) {
    srand(time(nullptr));

    for(int i=0;i<50;i++) {
        int sens = rand() % 3;
        int face = rand() % RUBIKSIZE;

        if(sens == 0) {
            rotate(face, CRubik::crrsX, 20000);
        } else if(sens == 1) {
            rotate(face + RUBIKSIZE, CRubik::crrsY, 20000);
        } else if(sens == 2) {
            rotate(face + 2 * RUBIKSIZE, CRubik::crrsZ, 20000);
        }
    }
}

void CRubik::init(void) {
    int x, y, z, i, j;

    for(z=i=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            for(x=0;x<RUBIKSIZE;x++,i++) {
                float fX = static_cast<float>(x-MARGIN);
                float fY = static_cast<float>(y-MARGIN);
                float fZ = static_cast<float>(z-MARGIN);

                float coords[NBFACE][NBSOMMET][DIMENSION] = {
                    { { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT } },
                    { { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY-UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT } },
                    { { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT } },
                    { { fX-UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ+UNIT } },
                    { { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX-UNIT, fY-UNIT, fZ+UNIT } },
                    { { fX-UNIT, fY+UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT } }
                };

                for(j=0;j<NBFACE;j++) {
                    QColor color = Qt::black;
                    CRubik::EFace face = static_cast<CRubik::EFace>(j);

                    if(x == 0 && face == crefOrange) {
                        color = QColor(0xff, 0x58, 0);
                    } else if(x == RUBIKSIZE - 1 && face == crefRouge) {
                        color = QColor(0xb7, 0x12, 0x34);
                    } else if(y == 0 && face == crefJaune) {
                        color = QColor(0xff, 0xd5, 0);
                    } else if(y == RUBIKSIZE - 1 && face == crefBlanc) {
                        color = Qt::white;
                    } else if(z == 0 && face == crefBlue) {
                       color = QColor(0, 0x46, 0xad);
                    } else if(z == RUBIKSIZE - 1 && face == crefVert) {
                        color = QColor(0, 0x9b, 0x48);
                    }

                    memcpy(&cubes[i].faces[j].coords, &coords[j], sizeof(float[NBSOMMET][DIMENSION]));
                    cubes[i].faces[j].color = color;
                    cubes[i].faces[j].colorFace = face;
                    cubes[i].xc = static_cast<int>(fX);
                    cubes[i].yc = static_cast<int>(fY);;
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

    memset(rGroupes, 0, sizeof(SCube *)*NBFACE*NBCUBEPARFACE);

    for(i=0;i<NBCUBE;i++) {
        int x = cubes[i].xc+MARGIN;
        int y = cubes[i].yc+MARGIN;
        int z = cubes[i].zc+MARGIN;

        groupex = rGroupes[x];
        groupey = rGroupes[y + RUBIKSIZE];
        groupez = rGroupes[z + 2 * RUBIKSIZE];

        groupex[z*RUBIKSIZE+y] = &cubes[i];
        groupey[z*RUBIKSIZE+x] = &cubes[i];
        groupez[y*RUBIKSIZE+x] = &cubes[i];
    }
}
