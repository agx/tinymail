#include <pygobject.h>
#include <pygtk/pygtk.h>

void tinymailui_gtk_register_classes (PyObject *d); 
extern PyMethodDef tinymailui_gtk_functions[];
 
DL_EXPORT(void)
inittinymailui_gtk(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
    init_pygtk();
    
    m = Py_InitModule ("tinymailui_gtk", tinymailui_gtk_functions);
    d = PyModule_GetDict (m);
 
    tinymailui_gtk_register_classes (d);
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module tinymailui_gtk");
    }
}
