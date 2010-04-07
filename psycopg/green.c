/* green.c - cooperation with coroutine libraries.
 *
 * Copyright (C) 2010 Daniele Varrazzo <daniele.varrazzo@gmail.com>
 *
 * This file is part of psycopg.
 *
 * psycopg2 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link this program with the OpenSSL library (or with
 * modified versions of OpenSSL that use the same license as OpenSSL),
 * and distribute linked combinations including the two.
 *
 * You must obey the GNU Lesser General Public License in all respects for
 * all of the code used other than OpenSSL.
 *
 * psycopg2 is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

#define PSYCOPG_MODULE
#include "psycopg/config.h"
#include "psycopg/python.h"
#include "psycopg/psycopg.h"
#include "psycopg/green.h"

HIDDEN PyObject *wait_callback = NULL;

/* Register a callback function to block waiting for data.
 *
 * The function is exported by the _psycopg module.
 */
PyObject *
psyco_set_wait_callback(PyObject *self, PyObject *obj)
{
    Py_XDECREF(wait_callback);

    if (obj != Py_None) {
        wait_callback = obj;
        Py_INCREF(obj);
    }
    else {
        wait_callback = NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


/* Return nonzero if a wait callback should be called. */
int
psyco_green()
{
#ifdef PSYCOPG_EXTENSIONS
    return (NULL != wait_callback);
#else
    return 0;
#endif
}

/* Block waiting for data available in an async connection.
 *
 * This function assumes `wait_callback` to be available:
 * raise `InterfaceError` if it is not. Use `psyco_green()` to check if
 * the function is to be called.
 *
 * The function returns the return value of the called function.
 */
PyObject *
psyco_wait(PyObject *conn, PyObject *curs)
{
    PyObject *rv;
    PyObject *cb;

    Dprintf("psyco_wait");
    cb = wait_callback;
    if (!cb) {
        PyErr_SetString(OperationalError, "wait callback not available");
        return NULL;
    }

    Py_INCREF(cb);
    rv = PyObject_CallFunctionObjArgs(cb, conn, curs, NULL);
    Py_DECREF(cb);

    return rv;
}
