#include <Python.h>
#include <QtDebug>
#include <QApplication>
#include "CMainWindow.h"

static PyObject * rubik_melange(PyObject *, PyObject *);
static PyObject * rubik_init(PyObject *, PyObject *);
static PyObject * rubik_exec(PyObject *, PyObject *);
static PyObject * rubik_map(PyObject *, PyObject *);
static PyObject * rubik_win(PyObject *, PyObject *);
static PyObject * rubik_debug(PyObject *, PyObject *);
static PyObject * PyInit_rubik(void);

static CRubik *rubik = new CRubik();
static PyMethodDef RubikMethods[] = {
    {"melange",  rubik_melange, METH_VARARGS, "Mélange le cube."},
    {"init",  rubik_init, METH_VARARGS, "Ré-initialise le cube."},
    {"exec",  rubik_exec, METH_VARARGS, "Exécute une série de mouvements."},
    {"map",  rubik_map, METH_VARARGS, "Retourne la map du cube."},
    {"win",  rubik_win, METH_VARARGS, "Retourne vrai si le cube est ok."},
    {"debug",  rubik_debug, METH_VARARGS, "Affiche les information d'un cube."},
    {nullptr, nullptr, 0, nullptr}
};
static struct PyModuleDef rubikmodule = {
    PyModuleDef_HEAD_INIT, "rubik", nullptr,  -1, RubikMethods
};

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    CMainWindow w(rubik);
    int result = 0;

    srand(static_cast<unsigned int>(time(nullptr)));

#if PY_VERSION_HEX >= 0x03070300
    wchar_t *program = Py_DecodeLocale(argv[0], nullptr);
    if (program == nullptr) {
        fprintf(stderr, "Erreur: impossible de décoder argv[0]\n");
        return 1;
    }
    Py_SetProgramName(program);
#endif

    PyImport_AppendInittab("rubik", &PyInit_rubik);
    Py_Initialize();
    PyImport_ImportModule("rubik");

    w.show();

    result = a.exec();

#if PY_VERSION_HEX >= 0x03070300
    PyMem_RawFree(program);
#endif

    return result;
}

PyObject * rubik_melange(PyObject *, PyObject *args) {
    int nb, p;

    if(!PyArg_ParseTuple(args, "ip", &nb, &p)) {
        return nullptr;
    }

    return PyUnicode_FromString(rubik->melange(nb, p == 1).toUtf8().data());
}

PyObject * rubik_init(PyObject *, PyObject *) {
    rubik->init();

    Py_INCREF(Py_None);

    return Py_None;
}

PyObject * rubik_exec(PyObject *, PyObject *args) {
    char *cmd;

    if(!PyArg_ParseTuple(args, "s", &cmd)) {
        return nullptr;
    }

    return PyUnicode_FromString(rubik->exec(QString(cmd)).toUtf8().data());
}

PyObject * rubik_map(PyObject *, PyObject *) {
    PyObject* listObj = PyList_New(RUBIKSIZE * RUBIKSIZE * NBFACE * NBFACE);
    int idx = 0;
    int x, y, z;


    if (!listObj) return nullptr;

    //X
    for(z=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            CRubik::EFace face = rubik->getFace(0, y, z, CMouvement::cmedX);
            for(int i=0;i<NBFACE;i++) {
                PyObject *num = PyLong_FromLong(i == static_cast<int>(face) ? 1 : 0);
                if(num != nullptr) {
                    PyList_SET_ITEM(listObj, idx++, num);
                } else {
                    Py_DECREF(listObj);
                    return nullptr;
                }
            }
        }
    }
    for(z=0;z<RUBIKSIZE;z++) {
        for(y=0;y<RUBIKSIZE;y++) {
            CRubik::EFace face = rubik->getFace(2, y, z, CMouvement::cmedX);
            for(int i=0;i<NBFACE;i++) {
                PyObject *num = PyLong_FromLong(i == static_cast<int>(face) ? 1 : 0);
                if(num != nullptr) {
                    PyList_SET_ITEM(listObj, idx++, num);
                } else {
                    Py_DECREF(listObj);
                    return nullptr;
                }
            }
        }
    }

    //Y
    for(x=RUBIKSIZE-1;x>=0;x--) {
        for(y=0;y<RUBIKSIZE;y++) {
            CRubik::EFace face = rubik->getFace(x, y, 0, CMouvement::cmedY);
            for(int i=0;i<NBFACE;i++) {
                PyObject *num = PyLong_FromLong(i == static_cast<int>(face) ? 1 : 0);
                if(num != nullptr) {
                    PyList_SET_ITEM(listObj, idx++, num);
                } else {
                    Py_DECREF(listObj);
                    return nullptr;
                }
            }
        }
    }

    for(x=RUBIKSIZE-1;x>=0;x--) {
        for(y=0;y<RUBIKSIZE;y++) {
            CRubik::EFace face = rubik->getFace(x, y, 2, CMouvement::cmedY);
            for(int i=0;i<NBFACE;i++) {
                PyObject *num = PyLong_FromLong(i == static_cast<int>(face) ? 1 : 0);
                if(num != nullptr) {
                    PyList_SET_ITEM(listObj, idx++, num);
                } else {
                    Py_DECREF(listObj);
                    return nullptr;
                }
            }
        }
    }

    //Z
    for(x=RUBIKSIZE-1;x>=0;x--) {
        for(z=0;z<RUBIKSIZE;z++) {
            CRubik::EFace face = rubik->getFace(x, 0, z, CMouvement::cmedZ);
            for(int i=0;i<NBFACE;i++) {
                PyObject *num = PyLong_FromLong(i == static_cast<int>(face) ? 1 : 0);
                if(num != nullptr) {
                    PyList_SET_ITEM(listObj, idx++, num);
                } else {
                    Py_DECREF(listObj);
                    return nullptr;
                }
            }
        }
    }

    for(x=RUBIKSIZE-1;x>=0;x--) {
        for(z=0;z<RUBIKSIZE;z++) {
            CRubik::EFace face = rubik->getFace(x, 2, z, CMouvement::cmedZ);
            for(int i=0;i<NBFACE;i++) {
                PyObject *num = PyLong_FromLong(i == static_cast<int>(face) ? 1 : 0);
                if(num != nullptr) {
                    PyList_SET_ITEM(listObj, idx++, num);
                } else {
                    Py_DECREF(listObj);
                    return nullptr;
                }
            }
        }
    }

    return listObj;
}

PyObject * rubik_win(PyObject *, PyObject *) {
    return rubik->win() ? Py_True : Py_False;
}

PyObject * rubik_debug(PyObject *, PyObject *args) {
    int x, y, z;

    if(!PyArg_ParseTuple(args, "iii", &x, &y, &z)) {
        return nullptr;
    }

    rubik->printCubeInfo(x, y, z);

    Py_INCREF(Py_None);

    return Py_None;
}

PyObject * PyInit_rubik(void) {
    return PyModule_Create(&rubikmodule);
}
