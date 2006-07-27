#include <pygobject.h>
 
void tinymail_platform_register_classes (PyObject *d); 
extern PyMethodDef tinymail_platform_functions[];
 
DL_EXPORT(void)
initotinymail_platform(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("tinymail_platform", tinymail_platform_functions);
    d = PyModule_GetDict (m);
 
    tinymail_platform_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module tinymail_platform");
    }
}
