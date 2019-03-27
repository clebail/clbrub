#include <sstream>
#include "CMouvement.h"
#include "CScanner.h"

static QChar groupeMap[] = { 'F', 'S', 'B', 'D', 'E', 'U', 'L', 'M', 'R' };

extern QList<CMouvement *> getResult();

CMouvement::CMouvement() {
    groupe = 0;
    sens = CMouvement::crrsX;
    inverse = false;
}

CMouvement::CMouvement(QChar type) {
    groupe = 0;
    while(groupeMap[groupe] != type) {
        groupe++;
        Q_ASSERT(groupe < 9);
    }
    sens = (groupe < 3 ? CMouvement::crrsX : (groupe < 6 ? CMouvement::crrsY : CMouvement::crrsZ));
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
    QList<CMouvement *> result;

    parser.parse();
    return getResult();
}

CMouvement *CMouvement::createMouvement(void) {
    int sens = rand() % 3;
    int face = rand() % RUBIKSIZE;
    CMouvement *mouvement = new CMouvement();

    mouvement->groupe = face + sens * RUBIKSIZE;
    mouvement->sens = static_cast<CMouvement::ERotate>(sens);
    mouvement->inverse = rand() % 2 == 1;

    return mouvement;
}
