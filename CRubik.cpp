#include <QtDebug>
#include <QThread>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "CRubik.h"

#define TAILLE_POPULATION                   100
#define TAILLE_GENOME                       25

// Couleurs standard des faces U R F D L B (ordre des valeurs de CCubeCore)
static const CRubik::EFace COULEURS[NBFACE] = { CRubik::crefBlanc, CRubik::crefRouge, CRubik::crefVert, CRubik::crefJaune, CRubik::crefOrange, CRubik::crefBlue };
// Couleur d'origine de chaque face d'un cubie (gauche, droite, derrière, devant, bas, haut : cf. calculCoords)
static const CRubik::EFace COULEURSCUBIE[NBFACE] = { CRubik::crefOrange, CRubik::crefRouge, CRubik::crefBlue, CRubik::crefVert, CRubik::crefJaune, CRubik::crefBlanc };

CRubik::CRubik(void) {   
    init();
}

const CRubik::SFace& CRubik::getSubFace(int idCube, int idFace) const {
    return cubes[idCube].faces[idFace];
}

QString CRubik::melange(int nb, bool anim, bool slices) {
    QStringList result;

    for(int i=0;i<nb;i++) {
        CMouvement *mouvement = CMouvement::createMouvement(slices);

        if(anim) {
            rotate(mouvement->getGroupe(), mouvement->getSens(), mouvement->getInverse(), ROTATE_STEP, 20);
        } else {
            rotate(mouvement->getGroupe(), mouvement->getSens(), mouvement->getInverse(), 1, 0);
        }

        result << *mouvement;

        delete mouvement;
    }

    return result.join(' ');
}

void CRubik::init(void) {
    int x, y, z, i, j;

    for(z=i=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            for(x=0;x<RUBIKSIZE;x++,i++) {
                float fX = static_cast<float>(x - MARGIN);
                float fY = static_cast<float>(y - MARGIN);
                float fZ = static_cast<float>(z - MARGIN);
                float coords[NBFACE][NBSOMMET][DIMENSION];

                calculCoords(fX, fY, fZ, coords);

                cubes[i].faces[0].origineOrientation = cubes[i].faces[1].origineOrientation = CMouvement::cmedX;
                cubes[i].faces[2].origineOrientation = cubes[i].faces[3].origineOrientation = CMouvement::cmedY;
                cubes[i].faces[4].origineOrientation = cubes[i].faces[5].origineOrientation = CMouvement::cmedZ;

                cubes[i].faces[0].orientation = cubes[i].faces[1].orientation = CMouvement::cmedX;
                cubes[i].faces[2].orientation = cubes[i].faces[3].orientation = CMouvement::cmedY;
                cubes[i].faces[4].orientation = cubes[i].faces[5].orientation = CMouvement::cmedZ;

                for(j=0;j<NBFACE;j++) {
                    CRubik::EFace face = COULEURSCUBIE[j];

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

    stickers = CCubeCore::solved();

    calculGroupes();

    emit(update());
    emit(endRotate());
}

QString CRubik::exec(QString cmd, bool anim) {
    QList<CMouvement *> mvts = CMouvement::formString(cmd);
    int i;

    // Commande vide ou invalide : rien à exécuter
    if(mvts.isEmpty()) {
        return "";
    }

    QString result = *mvts.last();

    for(i=0;i<mvts.size();i++) {
        CMouvement * mvt = mvts.at(i);

        //mouvements.append(mvt);
        if(anim) {
            rotate(mvt->getGroupe(), mvt->getSens(), mvt->getInverse());
        } else {
            rotate(mvt->getGroupe(), mvt->getSens(), mvt->getInverse(), 1, 0);
        }

        delete mvt;
    }

    return result;
}

void CRubik::printCubeInfo(int x, int y, int z) const {
    const SCube *cube = findCube(x, y, z);

    if(cube != nullptr) {
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
}

CRubik::EFace CRubik::getFace(int x, int y, int z, CMouvement::EDirection direction) const {
    // Axe géométrique (0 = x, 1 = y, 2 = z) de la normale associée à chaque EDirection
    static const int directionToAxe[DIMENSION] = { 0, 2, 1 };
    const int coords[DIMENSION] = { x, y, z };
    int axe = directionToAxe[direction];
    int signe = (coords[axe] == 0 ? -1 : (coords[axe] == RUBIKSIZE - 1 ? 1 : 0));
    int sticker = CCubeCore::stickerIndex(x, y, z, axe, signe);

    return sticker < 0 ? CRubik::crefBlack : getStickerColor(sticker);
}

bool CRubik::win(void) {
    return CCubeCore::isSolved(stickers.data());
}

CRubik::EFace CRubik::couleur(uint8_t valeur) {
    return valeur < NBFACE ? COULEURS[valeur] : CRubik::crefBlack;
}

uint8_t CRubik::valeur(CRubik::EFace couleur) {
    uint8_t i;

    if(couleur == CRubik::crefBlancClb) {
        couleur = CRubik::crefBlanc;
    }

    for(i=0;i<NBFACE && COULEURS[i]!=couleur;i++);

    return i;
}

QColor CRubik::fromEFace(CRubik::EFace colorFace) {
    switch(colorFace) {
    case CRubik::crefRouge:
        return QColor(0xb7, 0x12, 0x34);
    case CRubik::crefOrange:
        return QColor(0xff, 0x58, 0);
    case CRubik::crefBlue:
        return QColor(0, 0x46, 0xad);
    case CRubik::crefVert:
        return QColor(0, 0x9b, 0x48);
    case CRubik::crefJaune:
        return QColor(0xff, 0xd5, 0);
    case CRubik::crefBlanc:
    case CRubik::crefBlancClb:
        return Qt::white;
    default:
        return Qt::black;
    }
}

QByteArray CRubik::getState(void) const {
    return QByteArray(reinterpret_cast<const char *>(stickers.data()), CCubeCore::NBSTICKERS);
}

bool CRubik::setState(const QByteArray& state) {
    // Normale d'origine de chaque face d'un cubie (cf. calculCoords) : axe géométrique et signe
    static const int axeFace[NBFACE] = { 0, 0, 2, 2, 1, 1 };
    static const int signeFace[NBFACE] = { -1, 1, -1, 1, -1, 1 };
    const uint8_t *data = reinterpret_cast<const uint8_t *>(state.constData());
    int cubeParCouleurs[1 << NBFACE];
    bool utilise[NBCUBE] = { false };
    int position[NBCUBE][DIMENSION];
    int rotation[NBCUBE][DIMENSION][DIMENSION];
    int i, j, k, x, y, z;

    if(state.size() != CCubeCore::NBSTICKERS) {
        return false;
    }

    for(i=0;i<CCubeCore::NBSTICKERS;i++) {
        if(data[i] >= NBFACE) {
            return false;
        }
    }

    // Chaque cubie est identifié par l'ensemble de ses couleurs
    memset(cubeParCouleurs, -1, sizeof(cubeParCouleurs));
    for(i=0;i<NBCUBE;i++) {
        int masque = 0;

        for(j=0;j<NBFACE;j++) {
            if(cubes[i].faces[j].colorFace != CRubik::crefBlack) {
                masque |= 1 << valeur(cubes[i].faces[j].colorFace);
            }
        }

        cubeParCouleurs[masque] = i;
    }

    // Validation complète et calcul du placement avant toute modification du cube
    for(z=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            for(x=0;x<RUBIKSIZE;x++) {
                const int coords[DIMENSION] = { x, y, z };
                int normales[NBFACE][DIMENSION];
                int couleurs[NBFACE];
                int nb = 0, masque = 0;
                int r[DIMENSION][DIMENSION];
                bool connu[DIMENSION] = { false, false, false };
                int nbConnus = 0;

                // Autocollants présents à cette position
                for(j=0;j<DIMENSION;j++) {
                    for(k=-1;k<=1;k+=2) {
                        int sticker = CCubeCore::stickerIndex(x, y, z, j, k);

                        if(sticker >= 0) {
                            int p[DIMENSION];

                            CCubeCore::stickerGeometry(sticker, p, normales[nb]);
                            couleurs[nb] = data[sticker];

                            if(masque & (1 << couleurs[nb])) {
                                return false;
                            }

                            masque |= 1 << couleurs[nb];
                            nb++;
                        }
                    }
                }

                i = cubeParCouleurs[masque];
                if(i < 0 || utilise[i]) {
                    return false;
                }
                utilise[i] = true;

                // Chaque face colorée du cubie donne une colonne de la rotation : R * normale d'origine = normale actuelle
                for(j=0;j<NBFACE;j++) {
                    if(cubes[i].faces[j].colorFace != CRubik::crefBlack) {
                        int a = axeFace[j];

                        for(k=0;k<nb && couleurs[k]!=valeur(cubes[i].faces[j].colorFace);k++);

                        for(int l=0;l<DIMENSION;l++) {
                            r[l][a] = normales[k][l] * signeFace[j];
                        }

                        connu[a] = true;
                        nbConnus++;
                    }
                }

                if(nbConnus < 2) {
                    // Centre ou cubie intérieur : la rotation n'est pas entièrement déterminée par les couleurs,
                    // on garde la rotation actuelle si elle convient (le logo du centre blanc ne tourne pas inutilement)
                    int actuelle[DIMENSION][DIMENSION];
                    bool convient = true;

                    getRotation(&cubes[i], actuelle);

                    for(j=0;j<DIMENSION;j++) {
                        for(k=0;k<DIMENSION && connu[j];k++) {
                            convient = convient && (actuelle[k][j] == r[k][j]);
                        }
                    }

                    if(convient) {
                        memcpy(r, actuelle, sizeof(r));
                    } else {
                        // Sinon : un axe perpendiculaire quelconque complète la rotation
                        int a = (connu[0] ? 0 : (connu[1] ? 1 : 2));
                        int b = (a + 1) % DIMENSION;

                        for(j=0;j<DIMENSION;j++) {
                            for(k=0;k<DIMENSION;k++) {
                                r[k][b] = (k == j);
                            }
                            if(r[0][a] * r[0][b] + r[1][a] * r[1][b] + r[2][a] * r[2][b] == 0) {
                                break;
                            }
                        }
                        connu[b] = true;
                        nbConnus = 2;
                    }
                }

                if(nbConnus == 2) {
                    // Colonne manquante : R e_c = R e_(c+1) x R e_(c+2)
                    int c = (!connu[0] ? 0 : (!connu[1] ? 1 : 2));
                    int a = (c + 1) % DIMENSION;
                    int b = (c + 2) % DIMENSION;

                    r[0][c] = r[1][a] * r[2][b] - r[2][a] * r[1][b];
                    r[1][c] = r[2][a] * r[0][b] - r[0][a] * r[2][b];
                    r[2][c] = r[0][a] * r[1][b] - r[1][a] * r[0][b];
                }

                // Une rotation : colonnes unitaires (normales d'axes) et déterminant +1 (sinon arête/coin miroir)
                int det = r[0][0] * (r[1][1] * r[2][2] - r[1][2] * r[2][1])
                        - r[0][1] * (r[1][0] * r[2][2] - r[1][2] * r[2][0])
                        + r[0][2] * (r[1][0] * r[2][1] - r[1][1] * r[2][0]);

                if(det != 1) {
                    return false;
                }

                memcpy(rotation[i], r, sizeof(r));
                for(j=0;j<DIMENSION;j++) {
                    position[i][j] = coords[j] - MARGIN;
                }
            }
        }
    }

    placeCubes(position, rotation);
    memcpy(stickers.data(), data, CCubeCore::NBSTICKERS);

    calculGroupes();

    emit(update());
    emit(endRotate());

    return true;
}

void CRubik::placeCubes(const int position[NBCUBE][DIMENSION], const int rotation[NBCUBE][DIMENSION][DIMENSION]) {
    // Axe géométrique (0 = x, 1 = y, 2 = z) de la normale associée à chaque EDirection, et inversement
    static const int directionToAxe[DIMENSION] = { 0, 2, 1 };
    static const CMouvement::EDirection axeToDirection[DIMENSION] = { CMouvement::cmedX, CMouvement::cmedZ, CMouvement::cmedY };
    int i, j, k, l;

    for(i=0;i<NBCUBE;i++) {
        SCube *cube = &cubes[i];
        const int (*r)[DIMENSION] = rotation[i];
        float coords[NBFACE][NBSOMMET][DIMENSION];

        cube->xc = position[i][0];
        cube->yc = position[i][1];
        cube->zc = position[i][2];

        calculCoords(static_cast<float>(cube->xo), static_cast<float>(cube->yo), static_cast<float>(cube->zo), coords);

        for(j=0;j<NBFACE;j++) {
            SFace *face = &cube->faces[j];
            int axe = directionToAxe[face->origineOrientation];

            for(k=0;k<NBSOMMET;k++) {
                for(l=0;l<DIMENSION;l++) {
                    face->coords[k][l] = r[l][0] * coords[j][k][0] + r[l][1] * coords[j][k][1] + r[l][2] * coords[j][k][2];
                }
            }

            for(l=0;l<DIMENSION;l++) {
                if(r[l][axe] != 0) {
                    face->orientation = axeToDirection[l];
                }
            }
        }
    }
}

void CRubik::setDisplay(bool display) {
    blockSignals(!display);

    if(display) {
        emit(update());
        emit(endRotate());
    }
}

void CRubik::calculCoords(float fX, float fY, float fZ, float coords[NBFACE][NBSOMMET][DIMENSION]) {
    const float c[NBFACE][NBSOMMET][DIMENSION] = {
        { { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY-UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT } }, //gauche
        { { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT } }, //droite
        { { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT } }, //derrière
        { { fX-UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT }, { fX-UNIT, fY+UNIT, fZ+UNIT } }, //devant
        { { fX-UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY-UNIT, fZ-UNIT }, { fX+UNIT, fY-UNIT, fZ+UNIT }, { fX-UNIT, fY-UNIT, fZ+UNIT } }, //bas
        { { fX-UNIT, fY+UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ+UNIT }, { fX+UNIT, fY+UNIT, fZ-UNIT }, { fX-UNIT, fY+UNIT, fZ-UNIT } }  //haut
    };

    memcpy(coords, c, sizeof(c));
}

void CRubik::getRotation(const SCube *cube, int r[DIMENSION][DIMENSION]) {
    // Faces dont la normale d'origine est +x, +y et +z : leur normale actuelle donne les colonnes de la rotation
    static const int idFaces[DIMENSION] = { 1, 5, 3 };
    const int centre[DIMENSION] = { cube->xc, cube->yc, cube->zc };
    int j, k, l;

    for(k=0;k<DIMENSION;k++) {
        const SFace *face = &cube->faces[idFaces[k]];

        for(j=0;j<DIMENSION;j++) {
            float sum = 0.0f;

            for(l=0;l<NBSOMMET;l++) {
                sum += face->coords[l][j];
            }

            // Centre de la face - centre du cube = normale * UNIT ; l'arrondi absorbe la dérive des cos/sin
            r[j][k] = static_cast<int>(lroundf((sum / NBSOMMET - centre[j]) / UNIT));
        }
    }
}

void CRubik::calculGroupes(void) {
    int i;
    SCube **groupex;
    SCube **groupey;
    SCube **groupez;

    memset(rGroupes, 0, sizeof(rGroupes));
    memset(positions, 0, sizeof(positions));

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

        positions[(z * RUBIKSIZE + y) * RUBIKSIZE + x] = &cubes[i];
    }
}

void CRubik::rotate(int idRotateGroupe, CMouvement::EDirection rotateSens, bool inverse, int stepCount, unsigned int ts) {
    // Affichage coupé : animer ne servirait qu'à attendre
    if(signalsBlocked()) {
        stepCount = 1;
        ts = 0;
    }

    // La logique change immédiatement, les cubies suivent l'animation ; le sens du quart de tour vient du cœur
    int move = CCubeCore::moveFromGroupe(idRotateGroupe, inverse);
    CCubeCore::apply(stickers, move);

    int step;
    int coef = CCubeCore::coefficient(move);
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

                    // Les cubes du groupe permutent entre eux : la table reste cohérente une fois le groupe parcouru
                    positions[((cube->zc + MARGIN) * RUBIKSIZE + cube->yc + MARGIN) * RUBIKSIZE + cube->xc + MARGIN] = cube;
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
                                cube->faces[j].orientation = CMouvement::cmedY;
                            } else if(cube->faces[j].orientation == CMouvement::cmedY) {
                                cube->faces[j].orientation = CMouvement::cmedX;
                            }
                            break;
                        case CMouvement::cmedZ:
                            if(cube->faces[j].orientation == CMouvement::cmedZ) {
                                cube->faces[j].orientation = CMouvement::cmedX;
                            } else if(cube->faces[j].orientation == CMouvement::cmedX) {
                                cube->faces[j].orientation = CMouvement::cmedZ;
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

    emit(endRotate());
}

CRubik::SCube * CRubik::findCube(int x, int y, int z) const {
    if(x < 0 || x >= RUBIKSIZE || y < 0 || y >= RUBIKSIZE || z < 0 || z >= RUBIKSIZE) {
        return nullptr;
    }

    return positions[(z * RUBIKSIZE + y) * RUBIKSIZE + x];
}


