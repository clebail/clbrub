// Module Python autonome (sans Qt) exposant CCubeCore, pensé pour le travail par lots avec numpy.
// Compilation : python3 setup.py build_ext --inplace
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include <random>
#include <string>
#include <vector>
#include "CCubeCore.h"

// Convertit un objet en tableau uint8 contigu de forme (54,) ou (n, 54) et vérifie les couleurs
static PyArrayObject *toStates(PyObject *obj, npy_intp *n, bool *single) {
    PyArrayObject *states = reinterpret_cast<PyArrayObject *>(PyArray_FROMANY(obj, NPY_UINT8, 1, 2, NPY_ARRAY_IN_ARRAY | NPY_ARRAY_FORCECAST));

    if(states == nullptr) {
        return nullptr;
    }

    if(PyArray_DIM(states, PyArray_NDIM(states) - 1) != CCubeCore::NBSTICKERS) {
        PyErr_SetString(PyExc_ValueError, "Un état doit contenir 54 autocollants : forme (54,) ou (n, 54) attendue.");
        Py_DECREF(states);
        return nullptr;
    }

    *single = (PyArray_NDIM(states) == 1);
    *n = (*single ? 1 : PyArray_DIM(states, 0));

    const uint8_t *data = static_cast<const uint8_t *>(PyArray_DATA(states));
    for(npy_intp i=0;i<*n * CCubeCore::NBSTICKERS;i++) {
        if(data[i] > 5) {
            PyErr_SetString(PyExc_ValueError, "Les couleurs doivent être comprises entre 0 et 5.");
            Py_DECREF(states);
            return nullptr;
        }
    }

    return states;
}

static bool checkMove(npy_intp move) {
    if(move < 0 || move >= CCubeCore::NBMOVES) {
        PyErr_SetString(PyExc_ValueError, "Coup invalide (indice entre 0 et 17 attendu).");
        return false;
    }

    return true;
}

static PyObject *rubikcore_solved(PyObject *, PyObject *) {
    npy_intp dims[1] = { CCubeCore::NBSTICKERS };
    PyObject *result = PyArray_SimpleNew(1, dims, NPY_UINT8);

    if(result != nullptr) {
        memcpy(PyArray_DATA(reinterpret_cast<PyArrayObject *>(result)), CCubeCore::solved().data(), CCubeCore::NBSTICKERS);
    }

    return result;
}

static PyObject *rubikcore_moves(PyObject *, PyObject *args, PyObject *kwargs) {
    static const char *kwlist[] = { "slices", nullptr };
    int slices = 0;

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "|p", const_cast<char **>(kwlist), &slices)) {
        return nullptr;
    }

    PyObject *result = PyList_New(CCubeCore::nbMoves(slices));

    if(result == nullptr) {
        return nullptr;
    }

    for(int i=0;i<CCubeCore::nbMoves(slices);i++) {
        PyObject *name = PyUnicode_FromString(CCubeCore::moveName(i));

        if(name == nullptr) {
            Py_DECREF(result);
            return nullptr;
        }

        PyList_SET_ITEM(result, i, name);
    }

    return result;
}

static PyObject *rubikcore_parse(PyObject *, PyObject *args) {
    const char *str;
    std::vector<int> moves;

    if(!PyArg_ParseTuple(args, "s", &str)) {
        return nullptr;
    }

    if(!CCubeCore::parse(str, moves)) {
        PyErr_SetString(PyExc_ValueError, "Série de mouvements invalide (coups UDRLFBMES, suivis de ' et/ou d'un chiffre).");
        return nullptr;
    }

    PyObject *result = PyList_New(static_cast<Py_ssize_t>(moves.size()));

    if(result == nullptr) {
        return nullptr;
    }

    for(size_t i=0;i<moves.size();i++) {
        PyObject *move = PyLong_FromLong(moves[i]);

        if(move == nullptr) {
            Py_DECREF(result);
            return nullptr;
        }

        PyList_SET_ITEM(result, static_cast<Py_ssize_t>(i), move);
    }

    return result;
}

static PyObject *rubikcore_inverse(PyObject *, PyObject *args) {
    Py_ssize_t move;

    if(!PyArg_ParseTuple(args, "n", &move)) {
        return nullptr;
    }

    if(!checkMove(move)) {
        return nullptr;
    }

    return PyLong_FromLong(CCubeCore::inverse(static_cast<int>(move)));
}

static PyObject *rubikcore_apply(PyObject *, PyObject *args) {
    PyObject *statesObj, *movesObj;
    npy_intp n;
    bool single;
    std::vector<int> sequence;
    bool parMouvement = false;

    if(!PyArg_ParseTuple(args, "OO", &statesObj, &movesObj)) {
        return nullptr;
    }

    PyArrayObject *states = toStates(statesObj, &n, &single);
    if(states == nullptr) {
        return nullptr;
    }

    if(PyUnicode_Check(movesObj)) {
        // Une série de mouvements appliquée à chaque état
        const char *str = PyUnicode_AsUTF8(movesObj);

        if(str == nullptr || !CCubeCore::parse(str, sequence)) {
            if(str != nullptr) {
                PyErr_SetString(PyExc_ValueError, "Série de mouvements invalide (coups UDRLFBMES, suivis de ' et/ou d'un chiffre).");
            }
            Py_DECREF(states);
            return nullptr;
        }
    } else {
        // Un indice (même coup pour tous), une séquence d'indices (état seul) ou un indice par état (lot)
        PyArrayObject *brut = reinterpret_cast<PyArrayObject *>(PyArray_FROM_O(movesObj));

        if(brut == nullptr) {
            Py_DECREF(states);
            return nullptr;
        }

        // Indices entiers uniquement (une liste vide est vue par numpy comme un tableau de flottants)
        if(PyArray_NDIM(brut) > 1 || (!PyArray_ISINTEGER(brut) && PyArray_SIZE(brut) != 0)) {
            PyErr_SetString(PyExc_TypeError, "Coups attendus : indice entier, chaîne, ou séquence/tableau 1D d'indices entiers.");
            Py_DECREF(brut);
            Py_DECREF(states);
            return nullptr;
        }

        PyArrayObject *moves = reinterpret_cast<PyArrayObject *>(PyArray_FROMANY(reinterpret_cast<PyObject *>(brut), NPY_INTP, 0, 1, NPY_ARRAY_IN_ARRAY | NPY_ARRAY_FORCECAST));
        Py_DECREF(brut);

        if(moves == nullptr) {
            Py_DECREF(states);
            return nullptr;
        }

        const npy_intp *data = static_cast<const npy_intp *>(PyArray_DATA(moves));
        npy_intp nb = PyArray_SIZE(moves);

        parMouvement = (PyArray_NDIM(moves) == 1 && !single);
        if(parMouvement && nb != n) {
            PyErr_SetString(PyExc_ValueError, "Avec un lot d'états, il faut un coup par état (ou une chaîne pour une même série).");
            Py_DECREF(moves);
            Py_DECREF(states);
            return nullptr;
        }

        for(npy_intp i=0;i<nb;i++) {
            if(!checkMove(data[i])) {
                Py_DECREF(moves);
                Py_DECREF(states);
                return nullptr;
            }
            sequence.push_back(static_cast<int>(data[i]));
        }

        Py_DECREF(moves);
    }

    PyObject *result = PyArray_NewLikeArray(states, NPY_CORDER, nullptr, 0);
    if(result == nullptr) {
        Py_DECREF(states);
        return nullptr;
    }

    const uint8_t *in = static_cast<const uint8_t *>(PyArray_DATA(states));
    uint8_t *out = static_cast<uint8_t *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(result)));

    Py_BEGIN_ALLOW_THREADS
    for(npy_intp i=0;i<n;i++) {
        CCubeCore::State state;

        memcpy(state.data(), in + i * CCubeCore::NBSTICKERS, CCubeCore::NBSTICKERS);

        if(parMouvement) {
            CCubeCore::apply(state, sequence[static_cast<size_t>(i)]);
        } else {
            for(int move : sequence) {
                CCubeCore::apply(state, move);
            }
        }

        memcpy(out + i * CCubeCore::NBSTICKERS, state.data(), CCubeCore::NBSTICKERS);
    }
    Py_END_ALLOW_THREADS

    Py_DECREF(states);

    return result;
}

static PyObject *rubikcore_children(PyObject *, PyObject *args, PyObject *kwargs) {
    static const char *kwlist[] = { "states", "slices", nullptr };
    PyObject *statesObj;
    int slices = 0;
    npy_intp n;
    bool single;

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O|p", const_cast<char **>(kwlist), &statesObj, &slices)) {
        return nullptr;
    }

    PyArrayObject *states = toStates(statesObj, &n, &single);
    if(states == nullptr) {
        return nullptr;
    }

    npy_intp dims[3] = { n, CCubeCore::nbMoves(slices), CCubeCore::NBSTICKERS };
    PyObject *result = (single ? PyArray_SimpleNew(2, dims + 1, NPY_UINT8) : PyArray_SimpleNew(3, dims, NPY_UINT8));

    if(result != nullptr) {
        const uint8_t *in = static_cast<const uint8_t *>(PyArray_DATA(states));
        uint8_t *out = static_cast<uint8_t *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(result)));

        Py_BEGIN_ALLOW_THREADS
        CCubeCore::children(in, n, slices, out);
        Py_END_ALLOW_THREADS
    }

    Py_DECREF(states);

    return result;
}

static PyObject *rubikcore_is_solved(PyObject *, PyObject *args) {
    PyObject *statesObj;
    npy_intp n;
    bool single;

    if(!PyArg_ParseTuple(args, "O", &statesObj)) {
        return nullptr;
    }

    PyArrayObject *states = toStates(statesObj, &n, &single);
    if(states == nullptr) {
        return nullptr;
    }

    const uint8_t *in = static_cast<const uint8_t *>(PyArray_DATA(states));
    PyObject *result;

    if(single) {
        result = PyBool_FromLong(CCubeCore::isSolved(in));
    } else {
        npy_intp dims[1] = { n };

        result = PyArray_SimpleNew(1, dims, NPY_BOOL);
        if(result != nullptr) {
            npy_bool *out = static_cast<npy_bool *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(result)));

            Py_BEGIN_ALLOW_THREADS
            for(npy_intp i=0;i<n;i++) {
                out[i] = CCubeCore::isSolved(in + i * CCubeCore::NBSTICKERS);
            }
            Py_END_ALLOW_THREADS
        }
    }

    Py_DECREF(states);

    return result;
}

static PyObject *rubikcore_generate(PyObject *, PyObject *args, PyObject *kwargs) {
    static const char *kwlist[] = { "n", "k_max", "k_min", "slices", "seed", nullptr };
    Py_ssize_t n;
    int kMax, kMin = 1, slices = 0;
    PyObject *seedObj = Py_None;
    uint64_t seed;

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "ni|ipO", const_cast<char **>(kwlist), &n, &kMax, &kMin, &slices, &seedObj)) {
        return nullptr;
    }

    if(n < 0 || kMin < 0 || kMin > kMax) {
        PyErr_SetString(PyExc_ValueError, "Paramètres invalides : n >= 0 et 0 <= k_min <= k_max attendus.");
        return nullptr;
    }

    if(seedObj == Py_None) {
        std::random_device rd;

        seed = (static_cast<uint64_t>(rd()) << 32) ^ rd();
    } else {
        PyObject *index = PyNumber_Index(seedObj);

        if(index == nullptr) {
            return nullptr;
        }

        seed = PyLong_AsUnsignedLongLongMask(index);
        Py_DECREF(index);

        if(PyErr_Occurred()) {
            return nullptr;
        }
    }

    npy_intp dims[2] = { n, CCubeCore::NBSTICKERS };
    PyObject *states = PyArray_SimpleNew(2, dims, NPY_UINT8);
    PyObject *ks = PyArray_SimpleNew(1, dims, NPY_INT32);
    PyObject *last = PyArray_SimpleNew(1, dims, NPY_INT32);

    if(states == nullptr || ks == nullptr || last == nullptr) {
        Py_XDECREF(states);
        Py_XDECREF(ks);
        Py_XDECREF(last);
        return nullptr;
    }

    uint8_t *outStates = static_cast<uint8_t *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(states)));
    int32_t *outKs = static_cast<int32_t *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(ks)));
    int32_t *outLast = static_cast<int32_t *>(PyArray_DATA(reinterpret_cast<PyArrayObject *>(last)));

    Py_BEGIN_ALLOW_THREADS
    CCubeCore::generate(n, kMin, kMax, slices, seed, outStates, outKs, outLast);
    Py_END_ALLOW_THREADS

    return Py_BuildValue("(NNN)", states, ks, last);
}

static PyMethodDef RubikCoreMethods[] = {
    {"solved", rubikcore_solved, METH_NOARGS, "solved() -> ndarray (54,) uint8 : l'état résolu."},
    {"moves", reinterpret_cast<PyCFunction>(reinterpret_cast<void (*)(void)>(rubikcore_moves)), METH_VARARGS | METH_KEYWORDS,
     "moves(slices=False) -> list[str] : nom des coups, l'indice d'un coup étant sa position dans la liste."},
    {"parse", rubikcore_parse, METH_VARARGS, "parse(seq) -> list[int] : indices des coups d'une série (ex. \"R U'2 F\")."},
    {"inverse", rubikcore_inverse, METH_VARARGS, "inverse(move) -> int : indice du coup inverse (move ^ 1)."},
    {"apply", rubikcore_apply, METH_VARARGS,
     "apply(states, moves) -> ndarray : states de forme (54,) ou (n, 54) ; moves = indice, chaîne (même série pour tous), "
     "liste d'indices (série, pour un état seul) ou tableau (n,) (un coup par état)."},
    {"children", reinterpret_cast<PyCFunction>(reinterpret_cast<void (*)(void)>(rubikcore_children)), METH_VARARGS | METH_KEYWORDS,
     "children(states, slices=False) -> ndarray (n, 12, 54) (ou (12, 54)) : voisins de chaque état, dans l'ordre de moves()."},
    {"is_solved", rubikcore_is_solved, METH_VARARGS, "is_solved(states) -> bool ou ndarray (n,) bool."},
    {"generate", reinterpret_cast<PyCFunction>(reinterpret_cast<void (*)(void)>(rubikcore_generate)), METH_VARARGS | METH_KEYWORDS,
     "generate(n, k_max, k_min=1, slices=False, seed=None) -> (states (n, 54) uint8, ks (n,) int32, last (n,) int32) : "
     "n états mélangés de k coups (k uniforme dans [k_min, k_max]), sans coup annulant le précédent ; "
     "last = dernier coup joué (-1 si k = 0), son inverse résout le dernier pas."},
    {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef rubikcoremodule = {
    PyModuleDef_HEAD_INIT, "rubikcore", "Cœur logique du Rubik's cube (sans Qt), par lots avec numpy.", -1, RubikCoreMethods, nullptr, nullptr, nullptr, nullptr
};

PyMODINIT_FUNC PyInit_rubikcore(void) {
    import_array();

    PyObject *module = PyModule_Create(&rubikcoremodule);

    if(module != nullptr) {
        PyModule_AddIntConstant(module, "NB_STICKERS", CCubeCore::NBSTICKERS);
        PyModule_AddIntConstant(module, "NB_MOVES", CCubeCore::NBMOVES);
        PyModule_AddIntConstant(module, "NB_FACE_MOVES", CCubeCore::NBFACEMOVES);
    }

    return module;
}
