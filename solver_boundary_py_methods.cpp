#include "solver_boundary_py_methods.h"

#include <string>
#include <vector>

#include "solver_boundary_py_backend.h"

static bool py_requests_to_cpp(PyObject* obj, std::vector<SolverRequest>& out) {
    if (!PySequence_Check(obj)) {
        return false;
    }

    const Py_ssize_t n = PySequence_Size(obj);
    if (n < 0) {
        return false;
    }

    out.clear();
    out.reserve(static_cast<size_t>(n));

    for (Py_ssize_t i = 0; i < n; ++i) {
        PyObject* item = PySequence_GetItem(obj, i);
        if (!item) return false;

        if (!PyDict_Check(item)) {
            Py_DECREF(item);
            return false;
        }

        PyObject* type_obj = PyDict_GetItemString(item, "type");
        PyObject* challenge_obj = PyDict_GetItemString(item, "challenge");

        if (!type_obj || !challenge_obj ||
            !PyUnicode_Check(type_obj) || !PyUnicode_Check(challenge_obj)) {
            Py_DECREF(item);
            return false;
        }

        const char* type_c = PyUnicode_AsUTF8(type_obj);
        const char* challenge_c = PyUnicode_AsUTF8(challenge_obj);

        if (!type_c || !challenge_c) {
            Py_DECREF(item);
            return false;
        }

        SolverRequest req;
        req.type = type_c;
        req.challenge = challenge_c;
        out.push_back(std::move(req));

        Py_DECREF(item);
    }

    return true;
}

static PyObject* responses_to_py(const std::vector<SolverResponse>& responses) {
    PyObject* list = PyList_New(static_cast<Py_ssize_t>(responses.size()));
    if (!list) return nullptr;

    for (Py_ssize_t i = 0; i < static_cast<Py_ssize_t>(responses.size()); ++i) {
        const auto& r = responses[static_cast<size_t>(i)];

        PyObject* d = PyDict_New();
        if (!d) {
            Py_DECREF(list);
            return nullptr;
        }

        PyObject* type_obj = PyUnicode_FromString(r.type.c_str());
        PyObject* ok_obj = PyBool_FromLong(r.ok ? 1 : 0);
        PyObject* data_obj = PyUnicode_FromString(r.data.c_str());
        PyObject* err_obj = PyUnicode_FromString(r.error.c_str());

        if (!type_obj || !ok_obj || !data_obj || !err_obj) {
            Py_XDECREF(type_obj);
            Py_XDECREF(ok_obj);
            Py_XDECREF(data_obj);
            Py_XDECREF(err_obj);
            Py_DECREF(d);
            Py_DECREF(list);
            return nullptr;
        }

        int rc = 0;
        rc |= PyDict_SetItemString(d, "type", type_obj);
        rc |= PyDict_SetItemString(d, "ok", ok_obj);
        rc |= PyDict_SetItemString(d, "data", data_obj);
        rc |= PyDict_SetItemString(d, "error", err_obj);

        Py_DECREF(type_obj);
        Py_DECREF(ok_obj);
        Py_DECREF(data_obj);
        Py_DECREF(err_obj);

        if (rc != 0) {
            Py_DECREF(d);
            Py_DECREF(list);
            return nullptr;
        }

        PyList_SET_ITEM(list, i, d);
    }

    return list;
}

static PyObject* py_solve_requests(PyObject* self, PyObject* args, PyObject* kwargs) {
    (void)self;

    const char* player_path_c = nullptr;
    PyObject* requests_obj = nullptr;
    const char* solver_dir_c = ".";
    const char* meriyah_path_c = "meriyah.umd.js";
    const char* astring_path_c = "astring.min.js";

    static const char* kwlist[] = {
        "player_path",
        "requests",
        "solver_dir",
        "meriyah_path",
        "astring_path",
        nullptr
    };

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "sO|sss",
            const_cast<char**>(kwlist),
            &player_path_c,
            &requests_obj,
            &solver_dir_c,
            &meriyah_path_c,
            &astring_path_c)) {
        return nullptr;
    }

    try {
        std::vector<SolverRequest> requests;
        if (!py_requests_to_cpp(requests_obj, requests)) {
            PyErr_SetString(
                PyExc_TypeError,
                "requests must be a list of dicts with keys 'type' and 'challenge'"
            );
            return nullptr;
        }

        const auto responses = solve_requests_core(
            player_path_c,
            solver_dir_c,
            meriyah_path_c,
            astring_path_c,
            requests
        );

        return responses_to_py(responses);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "unknown error in solve_requests");
        return nullptr;
    }
}

static PyObject* py_solve_requests_from_watch(PyObject* self, PyObject* args, PyObject* kwargs) {
    (void)self;

    const char* watch_url_c = nullptr;
    PyObject* requests_obj = nullptr;
    const char* solver_dir_c = ".";
    const char* meriyah_path_c = "meriyah.umd.js";
    const char* astring_path_c = "astring.min.js";

    static const char* kwlist[] = {
        "watch_url",
        "requests",
        "solver_dir",
        "meriyah_path",
        "astring_path",
        nullptr
    };

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "sO|sss",
            const_cast<char**>(kwlist),
            &watch_url_c,
            &requests_obj,
            &solver_dir_c,
            &meriyah_path_c,
            &astring_path_c)) {
        return nullptr;
    }

    try {
        std::vector<SolverRequest> requests;
        if (!py_requests_to_cpp(requests_obj, requests)) {
            PyErr_SetString(
                PyExc_TypeError,
                "requests must be a list of dicts with keys 'type' and 'challenge'"
            );
            return nullptr;
        }

        const auto responses = solve_requests_from_watch_core(
            watch_url_c,
            solver_dir_c,
            meriyah_path_c,
            astring_path_c,
            requests
        );

        return responses_to_py(responses);
    } catch (const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return nullptr;
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "unknown error in solve_requests_from_watch");
        return nullptr;
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

PyMethodDef SolverBoundaryPyMethods[] = {
    {
        "solve_requests",
        (PyCFunction)py_solve_requests,
        METH_VARARGS | METH_KEYWORDS,
        "Load solver assets from a local player/base.js file and solve sig/n requests."
    },
    {
        "solve_requests_from_watch",
        (PyCFunction)py_solve_requests_from_watch,
        METH_VARARGS | METH_KEYWORDS,
        "Fetch current watch HTML/base.js dynamically and solve sig/n requests against the current player."
    },
    {nullptr, nullptr, 0, nullptr}
};

#pragma GCC diagnostic pop
