#include <sstream>
#include "CMouvement.h"
#include "CScanner.h"
#include "CCubeCore.h"


extern QList<CMouvement *> getResult();
extern void clearResult(void);

CMouvement::CMouvement() {
    groupe = 0;
    sens = CMouvement::cmedX;
    inverse = false;
}

CMouvement::CMouvement(QChar type) {
    // Lettres et couches définies par CCubeCore (notation standard)
    std::vector<int> moves;

    CCubeCore::parse(std::string(1, type.toLatin1()), moves);
    Q_ASSERT(moves.size() == 1);
    groupe = CCubeCore::groupe(moves.front());
    sens = (groupe < 3 ? CMouvement::cmedX : (groupe < 6 ? CMouvement::cmedY : CMouvement::cmedZ));
    inverse = false;
}

CMouvement::operator QString(void) const {
    return CCubeCore::moveName(CCubeCore::moveFromGroupe(groupe, inverse));
}

QList<CMouvement *> CMouvement::formString(QString str, bool *ok) {
    std::istringstream iss(str.toStdString());
    CScanner scanner(&iss);
    yy::CParser parser(scanner);

    clearResult();

    int erreur = parser.parse();

    if(ok != nullptr) {
        *ok = (erreur == 0);
    }

    return getResult();
}

QString CMouvement::toString(const QList<CMouvement *>& mouvements) {
    QStringList result;

    for(const CMouvement *mouvement : mouvements) {
        result << *mouvement;
    }

    return result.join(' ');
}

QString CMouvement::inverseSequence(QString str, bool *ok) {
    QList<CMouvement *> mouvements = formString(str, ok);
    QList<CMouvement *> inverses;
    QString result;

    // Inverse d'une séquence : coups pris dans l'ordre inverse, chacun inversé
    for(int i=mouvements.size()-1;i>=0;i--) {
        mouvements.at(i)->setInverse(!mouvements.at(i)->getInverse());
        inverses << mouvements.at(i);
    }

    result = toString(inverses);
    qDeleteAll(mouvements);

    return result;
}

QStringList CMouvement::liste(bool slices) {
    QStringList result;

    for(int i=0;i<CCubeCore::nbMoves(slices);i++) {
        result << CCubeCore::moveName(i);
    }

    return result;
}

CMouvement *CMouvement::createMouvement(bool slices) {
    int sens = rand() % DIMENSION;
    // Sans tranche, seules la première et la dernière couche de chaque axe sont tournées
    int face = (slices ? rand() % RUBIKSIZE : (rand() % 2) * (RUBIKSIZE - 1));
    CMouvement *mouvement = new CMouvement();

    mouvement->groupe = face + sens * RUBIKSIZE;
    mouvement->sens = static_cast<CMouvement::EDirection>(sens);
    mouvement->inverse = rand() % 2 == 1;

    return mouvement;
}
