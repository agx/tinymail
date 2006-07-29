#include <pygobject.h>
#include <pygtk/pygtk.h>

void pyuigtk_register_classes (PyObject *d); 
extern PyMethodDef pyuigtk_functions[];
 
DL_EXPORT(void)
inituigtk(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
    init_pygtk();
    
    m = Py_InitModule ("uigtk", pyuigtk_functions);
    d = PyModule_GetDict (m);
 
    pyuigtk_register_classes (d);
 
    if (PyErr_Occurred ()) {
        PyErr_Print();
        Py_FatalError ("can't initialise module uigtk");
    }
}
