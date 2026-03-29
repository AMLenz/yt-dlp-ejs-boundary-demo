#include <Python.h>

#include "solver_boundary_py_methods.h"

/*
 g++ -O3 -Wall -Wextra -shared -std=c++17 -fPIC \
   $(python3-config --cflags) \
   $(pkg-config --cflags jsoncpp) \
   solver_boundary_py.cpp \
   solver_boundary_py_backend.cpp \
   solver_boundary_py_methods.cpp \
   solver_engine.cpp \
   quickjs_json_utils.cpp \
   http_utils.cpp \
   -o solver_boundary_py$(python3-config --extension-suffix) \
   $(python3-config --ldflags) \
   $(pkg-config --libs jsoncpp) \
   -lssl -lcrypto \
   -I. -I quickjs -L quickjs -lquickjs
*/

static struct PyModuleDef solver_boundary_pymodule = {
    PyModuleDef_HEAD_INIT,
    "solver_boundary_py",
    nullptr,
    -1,
    SolverBoundaryPyMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

PyMODINIT_FUNC PyInit_solver_boundary_py(void) {
    return PyModule_Create(&solver_boundary_pymodule);
}
