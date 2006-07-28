#include <pygobject.h>
 
void pytinymail_register_classes (PyObject *d); 
extern PyMethodDef pytinymail_functions[];
 
DL_EXPORT(void)
init_tinymail(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("tinymail", pytinymail_functions);
    d = PyModule_GetDict (m);
 
    pytinymail_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module tinymail");
    }
}
