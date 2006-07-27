/* -*- Mode: C; c-basic-offset: 4 -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* include this first, before NO_IMPORT_PYGOBJECT is defined */
#include <pygobject.h>


void pytinymail_camel_register_classes (PyObject *d);
void pytinymail_camel_add_constants(PyObject *module, const gchar *strip_prefix);
extern PyMethodDef pytinymail_camel_functions[];

/* DL_EXPORT(void)
inittinymail_camel(void)
{
    PyObject *m, *d;
	
    init_pygobject ();
    
    m = Py_InitModule ("tinymail_camel", pytinymail_camel_functions);
    d = PyModule_GetDict (m);
	
    pytinymail_camel_register_classes (d);
	
	if (PyErr_Occurred())
        Py_FatalError("could not initialise module tinymail_camel");
} */
