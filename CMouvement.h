#ifndef CMOUVEMENT_H
#define CMOUVEMENT_H

#define DIMENSION                   3
#define MVTPAD                      3
#define RUBIKSIZE                   3

#include <QString>

class CMouvement {
public:
    typedef enum { cmedX, cmedY, cmedZ } EDirection;

    CMouvement();
    CMouvement(QChar type);
    operator QString(void) const;
    static CMouvement *createMouvement(void);
    inline int getGroupe(void) const { return groupe; }
    inline CMouvement::EDirection getSens(void) const { return sens; }
    inline bool getInverse(void) const { return inverse; }
    inline void setInverse(bool inverse) { this->inverse = inverse; }
    static QList<CMouvement *> formString(QString str);
private:
    int groupe;
    CMouvement::EDirection sens;
    bool inverse;
};

#endif // CMOUVEMENT_H
