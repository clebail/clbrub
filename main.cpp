#include <Python.h>
#include <QtDebug>
#include <QApplication>
#include "CMainWindow.h"

static PyObject * rubik_melange(PyObject *, PyObject *);
static PyObject * rubik_init(PyObject *, PyObject *);
static PyObject * rubik_exec(PyObject *, PyObject *);
static PyObject * rubik_distance(PyObject *, PyObject *);
static PyObject * rubik_lastmouvement(PyObject *, PyObject *);
static PyObject * rubik_debug(PyObject *, PyObject *);
PyMODINIT_FUNC PyInit_rubik(void);

static CRubik *rubik = new CRubik();
static PyMethodDef RubikMethods[] = {
    {"melange",  rubik_melange, METH_VARARGS, "Mélange le cube."},
    {"init",  rubik_init, METH_VARARGS, "Ré-initialise le cube."},
    {"exec",  rubik_exec, METH_VARARGS, "Exécute une série de mouvements."},
    {"distance",  rubik_distance, METH_VARARGS, "Calcul la distance d'un cube avec son emplacement final."},
    {"lastmouvement",  rubik_lastmouvement, METH_VARARGS, "Retourne le dernier mouvement."},
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

#if PY_VERSION_HEX >= 0x03070300
    wchar_t *program = Py_DecodeLocale(argv[0], nullptr);
    if (program == nullptr) {
        fprintf(stderr, "Erreur: impossible de décoder argv[0]\n");
        return 1;
    }
    Py_SetProgramName(program);
#else
    Py_SetProgramName(argv[0]);
#endif

    PyImport_AppendInittab("rubik", PyInit_rubik);
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

    if(PyArg_ParseTuple(args, "ip", &nb, &p)) {
        rubik->melange(nb, p == 1);
    }

    return Py_None;
}

PyObject * rubik_init(PyObject *, PyObject *) {
    rubik->init();

    return Py_None;
}

PyObject * rubik_exec(PyObject *, PyObject *args) {
    char *cmd;

    if(PyArg_ParseTuple(args, "s", &cmd)) {
        rubik->exec(QString(cmd));
    }

    return Py_None;
}

PyObject * rubik_distance(PyObject *, PyObject *args) {
    int x, y, z;

    if(PyArg_ParseTuple(args, "iii", &x, &y, &z)) {
        return PyLong_FromLong(rubik->distance(x, y, z));
    }

    return PyLong_FromLong(-1);
}

PyObject * rubik_lastmouvement(PyObject *, PyObject *) {
    QString mvt = rubik->getLastMouvement();

    return PyUnicode_FromString(mvt.toUtf8().data());
}

PyObject * rubik_debug(PyObject *, PyObject *args) {
    int x, y, z;

    if(PyArg_ParseTuple(args, "iii", &x, &y, &z)) {
        rubik->printCubeInfo(x, y, z);
    }

    return Py_None;
}

PyMODINIT_FUNC PyInit_rubik(void) {
    return PyModule_Create(&rubikmodule);
}
