#include <pygobject.h>
 
void tinymailui_register_classes (PyObject *d); 
extern PyMethodDef tinymailui_functions[];
 
DL_EXPORT(void)
inittinymailui(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("tinymailui", tinymailui_functions);
    d = PyModule_GetDict (m);
 
    tinymailui_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module tinymailui");
    }
}
