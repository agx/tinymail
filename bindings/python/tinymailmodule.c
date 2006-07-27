#include <pygobject.h>
 
void tinymail_register_classes (PyObject *d); 
extern PyMethodDef tinymail_functions[];
 
DL_EXPORT(void)
initotinymail(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("tinymail", tinymail_functions);
    d = PyModule_GetDict (m);
 
    tinymail_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module tinymail");
    }
}
