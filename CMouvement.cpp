#include <sstream>
#include "CMouvement.h"
#include "CScanner.h"

static QChar groupeMap[] = { 'F', 'S', 'B', 'D', 'E', 'U', 'L', 'M', 'R' };

extern QList<CMouvement *> getResult();
extern void clearResult(void);

CMouvement::CMouvement() {
    groupe = 0;
    sens = CMouvement::cmedX;
    inverse = false;
}

CMouvement::CMouvement(QChar type) {
    groupe = 0;
    while(groupeMap[groupe] != type) {
        groupe++;
        Q_ASSERT(groupe < 9);
    }
    sens = (groupe < 3 ? CMouvement::cmedX : (groupe < 6 ? CMouvement::cmedY : CMouvement::cmedZ));
    inverse = false;
}

CMouvement::operator QString(void) const {
    QString result = "";

    result += groupeMap[groupe];
    result += (inverse ? "'" : "");

    return result;
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
    // Même ordre que les labels de genTrain.py : faces puis tranches, chaque coup suivi de son inverse
    const QString types = (slices ? "UDRLFBMES" : "UDRLFB");
    QStringList result;

    for(const QChar& type : types) {
        result << QString(type) << QString(type) + "'";
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
