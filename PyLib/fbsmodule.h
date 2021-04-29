#include <Python.h>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/FileThread.h>
#include <FEBioStudio/ModelDocument.h>
#include <MeshIO/FileReader.h>
#include <iostream>

typedef struct {
    PyObject_HEAD
    CDocument* doc;
    /* Type-specific fields go here. */
} DocumentObject;

static PyTypeObject pyDocumentType = {
    PyVarObject_HEAD_INIT(NULL, 0)};

static void pyDocumentType_init()
{
    pyDocumentType.tp_name = "fbs.Document";
    pyDocumentType.tp_doc = "Document class";
    pyDocumentType.tp_basicsize = sizeof(DocumentObject);
    pyDocumentType.tp_itemsize = 0;
    pyDocumentType.tp_flags = Py_TPFLAGS_DEFAULT;
    pyDocumentType.tp_new = PyType_GenericNew;
}



//// I/O

static PyObject*
pyOpenFile(PyObject *self, PyObject *args)
{
    const char *fileName;

    if(!PyArg_ParseTuple(args, "s", &fileName))
        return NULL;

    PRV::getMainWindow()->OpenFile(fileName);

    Py_RETURN_NONE;
}

static PyObject*
pyImportFile(PyObject *self, PyObject *args)
{
    const char *fileName;

    if(!PyArg_ParseTuple(args, "s", &fileName))
        return NULL;

    auto wnd = PRV::getMainWindow();
    auto doc = wnd->GetDocument();
    FileReader* fileReader = wnd->CreateFileReader(fileName);

    QueuedFile qfile(doc, fileName, fileReader, 0);

    wnd->ReadFile(qfile);

    Py_RETURN_NONE;
}

static PyObject*
pyNewDocument(PyObject *self, PyObject *args)
{
    const char *fileName;

    if(!PyArg_ParseTuple(args, "s", &fileName))
        fileName = "New Model";

    auto wnd = PRV::getMainWindow();
    auto doc = wnd->CreateNewDocument();
    doc->SetDocTitle(fileName);
    wnd->SetActiveDocument(doc);

    Py_RETURN_NONE;
}

static PyMethodDef fbsMethods[] = {
    {"openFile", pyOpenFile, METH_VARARGS, "Open the specified file in FEBio Studio."},
    {"importFile", pyImportFile, METH_VARARGS,"Import the specified file in FEBio Studio."},
    {"newDocument", pyNewDocument, METH_VARARGS,"Create a new document specified file in FEBio Studio."},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef fbsModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "fbs",
    .m_doc = "Example module that creates an extension type.",
    .m_size = -1,
    .m_methods = fbsMethods,
};

PyMODINIT_FUNC
PyInit_fbs(void)
{
    PyObject *m;

    pyDocumentType_init();


    if (PyType_Ready(&pyDocumentType) < 0)
        return NULL;

    m = PyModule_Create(&fbsModule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&pyDocumentType);
    if (PyModule_AddObject(m, "Document", (PyObject *) &pyDocumentType) < 0) {
        Py_DECREF(&pyDocumentType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}