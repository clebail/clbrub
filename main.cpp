#define PY_SSIZE_T_CLEAN
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
static PyObject * rubik_get_state(PyObject *, PyObject *);
static PyObject * rubik_set_state(PyObject *, PyObject *);
static PyObject * rubik_display(PyObject *, PyObject *);
static PyObject * rubik_moves(PyObject *, PyObject *);
static PyObject * rubik_inverse(PyObject *, PyObject *);
static PyObject * rubik_seed(PyObject *, PyObject *);
static PyObject * PyInit_rubik(void);

static CRubik *rubik = new CRubik();
static PyMethodDef RubikMethods[] = {
    {"melange",  rubik_melange, METH_VARARGS, "Mélange le cube (nb, anim, slices=True) et retourne la séquence jouée."},
    {"init",  rubik_init, METH_VARARGS, "Ré-initialise le cube."},
    {"exec",  rubik_exec, METH_VARARGS, "Exécute une série de mouvements (anim=True par défaut)."},
    {"map",  rubik_map, METH_VARARGS, "Retourne la map du cube."},
    {"win",  rubik_win, METH_VARARGS, "Retourne vrai si le cube est ok."},
    {"debug",  rubik_debug, METH_VARARGS, "Affiche les information d'un cube."},
    {"get_state",  rubik_get_state, METH_NOARGS, "Retourne l'état du cube : 54 couleurs (bytes hashable, même format que rubikcore)."},
    {"set_state",  rubik_set_state, METH_VARARGS, "Place le cube dans un état (54 couleurs, bytes ou tableau numpy uint8)."},
    {"display",  rubik_display, METH_VARARGS, "Active/désactive le rafraîchissement de l'affichage (et les animations)."},
    {"moves",  rubik_moves, METH_VARARGS, "Retourne la liste des coups (slices=True inclut M, E et S)."},
    {"inverse",  rubik_inverse, METH_VARARGS, "Retourne la séquence inverse d'une série de mouvements."},
    {"seed",  rubik_seed, METH_VARARGS, "Initialise le générateur aléatoire des mélanges."},
    {nullptr, nullptr, 0, nullptr}
};
static struct PyModuleDef rubikmodule = {
    PyModuleDef_HEAD_INIT, "rubik", nullptr,  -1, RubikMethods, nullptr, nullptr, nullptr, nullptr
};

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    CMainWindow w(rubik);
    int result = 0;
    PyConfig config;
    PyStatus status;

    srand(static_cast<unsigned int>(time(nullptr)));

    PyImport_AppendInittab("rubik", &PyInit_rubik);

    PyConfig_InitPythonConfig(&config);
    status = PyConfig_SetBytesString(&config, &config.program_name, argv[0]);
    if(!PyStatus_Exception(status)) {
        status = Py_InitializeFromConfig(&config);
    }
    PyConfig_Clear(&config);

    if(PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
    }

    PyImport_ImportModule("rubik");

    // Libère le GIL pour que les threads QtConcurrent puissent l'acquérir
    // via PyGILState_Ensure() (cf. CMainWindow::runScript).
    PyThreadState *mainThreadState = PyEval_SaveThread();

    w.show();

    result = a.exec();

    PyEval_RestoreThread(mainThreadState);

    return result;
}

PyObject * rubik_melange(PyObject *, PyObject *args) {
    int nb, p;
    int slices = 1;

    if(!PyArg_ParseTuple(args, "ip|p", &nb, &p, &slices)) {
        return nullptr;
    }

    return PyUnicode_FromString(rubik->melange(nb, p == 1, slices == 1).toUtf8().data());
}

PyObject * rubik_init(PyObject *, PyObject *) {
    rubik->init();

    Py_INCREF(Py_None);

    return Py_None;
}

PyObject * rubik_exec(PyObject *, PyObject *args) {
    char *cmd;
    int anim = 1;

    if(!PyArg_ParseTuple(args, "s|p", &cmd, &anim)) {
        return nullptr;
    }

    return PyUnicode_FromString(rubik->exec(QString(cmd), anim == 1).toUtf8().data());
}

PyObject * rubik_map(PyObject *, PyObject *) {
    const CCubeCore::State& stickers = rubik->getStickers();
    PyObject* listObj = PyList_New(CCubeCore::NBSTICKERS * NBFACE);
    int idx = 0;

    if (!listObj) return nullptr;

    // Encodage one-hot des 54 autocollants
    for(int sticker=0;sticker<CCubeCore::NBSTICKERS;sticker++) {
        for(int i=0;i<NBFACE;i++) {
            PyObject *num = PyLong_FromLong(i == stickers[static_cast<size_t>(sticker)] ? 1 : 0);
            if(num != nullptr) {
                PyList_SET_ITEM(listObj, idx++, num);
            } else {
                Py_DECREF(listObj);
                return nullptr;
            }
        }
    }

    return listObj;
}

PyObject * rubik_win(PyObject *, PyObject *) {
    if(rubik->win()) {
        Py_RETURN_TRUE;
    }

    Py_RETURN_FALSE;
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

PyObject * rubik_get_state(PyObject *, PyObject *) {
    QByteArray state = rubik->getState();

    return PyBytes_FromStringAndSize(state.constData(), state.size());
}

PyObject * rubik_set_state(PyObject *, PyObject *args) {
    Py_buffer buffer;
    bool ok;

    // Tout objet buffer contigu : bytes, bytearray, ligne d'un tableau numpy uint8...
    if(!PyArg_ParseTuple(args, "y*", &buffer)) {
        return nullptr;
    }

    ok = rubik->setState(QByteArray(static_cast<const char *>(buffer.buf), static_cast<int>(buffer.len)));
    PyBuffer_Release(&buffer);

    if(!ok) {
        PyErr_SetString(PyExc_ValueError, "État du cube invalide (54 couleurs 0..5 attendues).");
        return nullptr;
    }

    Py_RETURN_NONE;
}

PyObject * rubik_display(PyObject *, PyObject *args) {
    int display;

    if(!PyArg_ParseTuple(args, "p", &display)) {
        return nullptr;
    }

    rubik->setDisplay(display == 1);

    Py_RETURN_NONE;
}

PyObject * rubik_moves(PyObject *, PyObject *args) {
    int slices = 1;

    if(!PyArg_ParseTuple(args, "|p", &slices)) {
        return nullptr;
    }

    QStringList moves = CMouvement::liste(slices == 1);
    PyObject *listObj = PyList_New(moves.size());

    if (!listObj) return nullptr;

    for(int i=0;i<moves.size();i++) {
        PyObject *move = PyUnicode_FromString(moves.at(i).toUtf8().data());

        if(move == nullptr) {
            Py_DECREF(listObj);
            return nullptr;
        }

        PyList_SET_ITEM(listObj, i, move);
    }

    return listObj;
}

PyObject * rubik_inverse(PyObject *, PyObject *args) {
    char *cmd;
    bool ok;

    if(!PyArg_ParseTuple(args, "s", &cmd)) {
        return nullptr;
    }

    QString result = CMouvement::inverseSequence(QString(cmd), &ok);

    if(!ok) {
        PyErr_SetString(PyExc_ValueError, "Série de mouvements invalide.");
        return nullptr;
    }

    return PyUnicode_FromString(result.toUtf8().data());
}

PyObject * rubik_seed(PyObject *, PyObject *args) {
    unsigned int seed;

    if(!PyArg_ParseTuple(args, "I", &seed)) {
        return nullptr;
    }

    srand(seed);

    Py_RETURN_NONE;
}

PyObject * PyInit_rubik(void) {
    return PyModule_Create(&rubikmodule);
}
