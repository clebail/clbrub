#include <Python.h>
#include <QApplication>
#include "CMainWindow.h"

static PyObject * rubik_melange(PyObject *, PyObject *);
static PyObject * rubik_init(PyObject *, PyObject *);
static PyObject * rubik_exec(PyObject *, PyObject *);
static PyObject * rubik_distance(PyObject *, PyObject *);
PyMODINIT_FUNC PyInit_rubik(void);

static CRubik *rubik = new CRubik();
static PyMethodDef RubikMethods[] = {
    {"melange",  rubik_melange, METH_VARARGS, "Mélange le cube."},
    {"init",  rubik_init, METH_VARARGS, "Ré-initialise le cube."},
    {"exec",  rubik_exec, METH_VARARGS, "Exécute une série de mouvements."},
    {"distance",  rubik_distance, METH_VARARGS, "Calcul la distance d'un cube avec son emplacement final."},
    {nullptr, nullptr, 0, nullptr}
};
static struct PyModuleDef rubikmodule = {
    PyModuleDef_HEAD_INIT, "rubik", nullptr,  -1, RubikMethods
};

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    CMainWindow w(rubik);
    wchar_t *program = Py_DecodeLocale(argv[0], nullptr);
    int result = 0;

    if (program == nullptr) {
        fprintf(stderr, "Erreur: impossible de décoder argv[0]\n");
        return 1;
    }

    PyImport_AppendInittab("rubik", PyInit_rubik);
    Py_SetProgramName(program);
    Py_Initialize();
    PyImport_ImportModule("rubik");

    w.show();

    result = a.exec();

    PyMem_RawFree(program);

    return result;
}

PyObject * rubik_melange(PyObject *, PyObject *) {
    rubik->melange();

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

PyMODINIT_FUNC PyInit_rubik(void) {
    return PyModule_Create(&rubikmodule);
}
