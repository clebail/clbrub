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

QList<CMouvement *> CMouvement::formString(QString str) {
    std::istringstream iss(str.toStdString());
    CScanner scanner(&iss);
    yy::CParser parser(scanner);

    clearResult();

    parser.parse();
    return getResult();
}

CMouvement *CMouvement::createMouvement(void) {
    int sens = rand() % DIMENSION;
    int face = rand() % RUBIKSIZE;
    CMouvement *mouvement = new CMouvement();

    mouvement->groupe = face + sens * RUBIKSIZE;
    mouvement->sens = static_cast<CMouvement::EDirection>(sens);
    mouvement->inverse = rand() % 2 == 1;

    return mouvement;
}
