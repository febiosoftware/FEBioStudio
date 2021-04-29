#include "RunPython.h"
#include "fbsmodule.h"
#include <stdio.h>



void runPython(const char* scriptName)
{
    FILE* file;

    PyImport_AppendInittab("fbs", &PyInit_fbs);
    // PyImport_AppendInittab("doc", &PyInit_custom);
    Py_Initialize();
    file = fopen(scriptName, "r");
    PyRun_SimpleFile(file, scriptName);
    Py_Finalize();
}