#ifndef CMOUVEMENT_H
#define CMOUVEMENT_H

#define MVTPAD                      3
#define RUBIKSIZE                   3

#include <QString>

class CMouvement {
public:
    typedef enum { crrsX, crrsY, crrsZ } ERotate;

    CMouvement();
    CMouvement(QChar type);
    operator QString(void) const;
    static CMouvement createMouvement(void);
    inline int getGroupe(void) const { return groupe; }
    inline CMouvement::ERotate getSens(void) const { return sens; }
    inline bool getInverse(void) const { return inverse; }
    inline void setInverse(bool inverse) { this->inverse = inverse; }
    static QList<CMouvement *> formString(QString str);
private:
    int groupe;
    CMouvement::ERotate sens;
    bool inverse;
};

#endif // CMOUVEMENT_H
