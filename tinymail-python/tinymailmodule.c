/* -*- Mode: C; c-basic-offset: 4 -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* include this first, before NO_IMPORT_PYGOBJECT is defined */
#include <pygobject.h>


void pytinymail_register_classes (PyObject *d);
void pytinymail_add_constants(PyObject *module, const gchar *strip_prefix);
extern PyMethodDef pytinymail_functions[];

DL_EXPORT(void)
inittinymail(void)
{
    PyObject *m, *d;
	
    init_pygobject ();
    
    m = Py_InitModule ("tinymail", pytinymail_functions);
    d = PyModule_GetDict (m);
	
    pytinymail_register_classes (d);
	
	if (PyErr_Occurred())
        Py_FatalError("could not initialise module tinymail");
}
