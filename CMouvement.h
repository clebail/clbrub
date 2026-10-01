#ifndef CMOUVEMENT_H
#define CMOUVEMENT_H

#define DIMENSION                   3
#define MVTPAD                      3
#define RUBIKSIZE                   3

#include <QString>
#include <QStringList>

class CMouvement {
public:
    typedef enum { cmedX, cmedY, cmedZ } EDirection;

    CMouvement();
    CMouvement(QChar type);
    operator QString(void) const;
    static CMouvement *createMouvement(bool slices = true);
    inline int getGroupe(void) const { return groupe; }
    inline CMouvement::EDirection getSens(void) const { return sens; }
    inline bool getInverse(void) const { return inverse; }
    inline void setInverse(bool inverse) { this->inverse = inverse; }
    static QList<CMouvement *> formString(QString str, bool *ok = nullptr);
    static QString toString(const QList<CMouvement *>& mouvements);
    static QString inverseSequence(QString str, bool *ok = nullptr);
    static QStringList liste(bool slices = true);
private:
    int groupe;
    CMouvement::EDirection sens;
    bool inverse;
};

#endif // CMOUVEMENT_H
